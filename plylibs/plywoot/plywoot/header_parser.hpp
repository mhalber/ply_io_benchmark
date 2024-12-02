/*
   This file is part of PLYwoot, a header-only PLY parser.

   Copyright (C) 2023-2024, Ton van den Heuvel

   PLYwoot is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PLYWOOT_HEADER_PARSER_HPP
#define PLYWOOT_HEADER_PARSER_HPP

/// \file

#include "exceptions.hpp"
#include "header_scanner.hpp"
#include "header_scanner_ios.hpp"
#include "std.hpp"
#include "types.hpp"

#include <cassert>
#include <cstdint>
#include <istream>
#include <string>
#include <utility>
#include <vector>

namespace plywoot {

/// Base class for all header parser exceptions.
struct HeaderParserException : Exception
{
  /// Constructs a header parser exception with the given error message.
  ///
  /// \param message header parser error message
  HeaderParserException(const std::string &message) : Exception("parser error: " + message) {}
};

/// Exception thrown in case some invalid format specification is found in the
/// input.
struct InvalidFormat : HeaderParserException
{
  /// Constructs an invalid format parser exception with the given error
  /// message.
  ///
  /// \param format the invalid format keyword that was found in the header
  InvalidFormat(const std::string &format) : HeaderParserException("invalid format found: " + format) {}
};

/// Exception thrown in case the input contains an unexpected token.
struct UnexpectedToken : HeaderParserException
{
  /// Constructs an unexpected token header parser exception.
  ///
  /// \param expected token that was expected
  /// \param found token that was found
  /// \param tokenString textual representation of the token that was found
  UnexpectedToken(
      detail::HeaderScanner::Token expected,
      detail::HeaderScanner::Token found,
      const std::string &tokenString)
      : HeaderParserException(
            "unexpected token '" + detail::to_string(found) + "' found, expected '" +
            detail::to_string(expected) + "' (=" + tokenString + ") instead"),
        expected_{expected},
        found_{found}
  {
  }

  /// Constructs an unexpected token header parser exception for cases where the
  /// expected token is not clearly defined.
  ///
  /// \param found token that was found
  /// \param tokenString textual representation of the token that was found
  // TODO(ton): fix passing Eof below as the expected token
  UnexpectedToken(detail::HeaderScanner::Token found, const std::string &tokenString)
      : UnexpectedToken{detail::HeaderScanner::Token::Eof, found, tokenString}
  {
  }

  /// Returns the expected token.
  ///
  /// \return the expected token
  detail::HeaderScanner::Token expected() const { return expected_; }
  /// Returns the token that was found instead of the expected token.
  ///
  /// \return the token that was found instead of the expected token
  detail::HeaderScanner::Token found() const { return found_; }

private:
  /// Expected token.
  detail::HeaderScanner::Token expected_;
  /// Token that was found instead of the expected token.
  detail::HeaderScanner::Token found_;
};

}

namespace plywoot::detail {

/// Parser implementation for a PLY header. Results in a list of PLY element
/// specifications.
class HeaderParser
{
  using Token = detail::HeaderScanner::Token;

public:
  /// Constructs a header parser for the PLY header data in the given input
  /// stream.
  ///
  /// \param is input stream containing PLY header data to parse
  HeaderParser(std::istream &is) : scanner_{is}
  {
    accept(Token::MagicNumber);

    // Parse the format section.
    accept(Token::Format);
    switch (scanner_.nextToken())
    {
      case Token::Ascii:
        format_ = PlyFormat::Ascii;
        break;
      case Token::BinaryLittleEndian:
        format_ = PlyFormat::BinaryLittleEndian;
        break;
      case Token::BinaryBigEndian:
        format_ = PlyFormat::BinaryBigEndian;
        break;
      default:
        throw InvalidFormat(scanner_.tokenString());
    }
    scanner_.nextToken();  // ignore the format version

    // Ignore comment section for now.
    while (scanner_.nextToken() == Token::Comment) { comments_.push_back(scanner_.comment()); }

    // Parse elements.
    do {
      switch (scanner_.token())
      {
        case Token::EndHeader:
          break;
        case Token::Element:
          elements_.push_back(parseElement());
          break;
        case Token::Comment:
          comments_.push_back(scanner_.comment());
          scanner_.nextToken();
          break;
        default:
          throw UnexpectedToken(scanner_.token(), scanner_.tokenString());
          break;
      }
    } while (scanner_.token() != Token::EndHeader);
  }

  /// Returns all comments that were extracted from the PLY header by this
  /// parser.
  ///
  /// \return all comments in the PLY header parsed by this parser
  const std::vector<Comment> &comments() const { return comments_; }
  /// Returns all PLY elements defined in the parsed PLY header.
  ///
  /// \return all PLY elements defined in the parsed PLY header
  const std::vector<PlyElement> &elements() const { return elements_; }

  /// Returns the PLY format type as encoded in the PLY header.
  ///
  /// \return the PLY format type as encoded in the PLY header
  PlyFormat format() const { return format_; }

private:
  /// Asks the scanner for the next token, and verifies that it matches the
  /// given expected token. In case it fails, throws `UnexpectedToken`.
  void accept(Token expected)
  {
    if (scanner_.nextToken() != expected)
    {
      // In case an identifier token is expected, all reserved keywords are
      // acceptable as well.
      if (!(expected == Token::Identifier && scanner_.isKeyword(scanner_.token())))
      {
        throw UnexpectedToken(expected, scanner_.token(), scanner_.tokenString());
      }
    }
  }

  /// Converts a scanner token type to a data type, in case the token
  /// represents a data type. Otherwise, this throws `UnexpectedToken`.
  PlyDataType tokenToDataType(Token t) const
  {
    switch (t)
    {
      case Token::Char:
        return PlyDataType::Char;
      case Token::UChar:
        return PlyDataType::UChar;
      case Token::Short:
        return PlyDataType::Short;
      case Token::UShort:
        return PlyDataType::UShort;
      case Token::Int:
        return PlyDataType::Int;
      case Token::UInt:
        return PlyDataType::UInt;
      case Token::Float:
        return PlyDataType::Float;
      case Token::Double:
        return PlyDataType::Double;
      default:
        // TODO(ton): expected is a range of tokens...
        throw UnexpectedToken(Token::Char, t, scanner_.tokenString());
    }
  }

  /// Parses an element definition together with its associated properties.
  PlyElement parseElement()
  {
    accept(Token::Identifier);  // name of the elements
    std::string name{scanner_.tokenString()};

    accept(Token::Number);
    std::size_t size{scanner_.tokenNumber()};

    PlyElement result{std::move(name), size};

    // Parse properties.
    while (scanner_.nextToken() == Token::Property || scanner_.token() == Token::Comment)
    {
      if (scanner_.token() == Token::Property)
      {
        PlyDataType type;
        PlyDataType sizeType;

        // TODO(ton): probably reserved keywords may be used as names as well,
        // just accept every token here, even numbers?
        switch (scanner_.nextToken())
        {
          case Token::List:
            sizeType = tokenToDataType(scanner_.nextToken());
            type = tokenToDataType(scanner_.nextToken());
            accept(Token::Identifier);
            result.addProperty(scanner_.tokenString(), type, sizeType);
            break;
          default:
            type = tokenToDataType(scanner_.token());
            accept(Token::Identifier);
            result.addProperty(scanner_.tokenString(), type);
            break;
        }
      }
      else
      {
        assert(scanner_.token() == Token::Comment);
        comments_.push_back(scanner_.comment());
      }
    }

    return result;
  }

  /// Comment line in the header.
  std::vector<Comment> comments_;
  /// Format the data is stored in.
  PlyFormat format_;
  /// PLY elements defined in the header.
  std::vector<PlyElement> elements_;
  /// Reference to the wrapped input stream.
  detail::HeaderScanner scanner_;
};

}

#endif
