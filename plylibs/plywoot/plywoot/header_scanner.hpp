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

#ifndef PLYWOOT_HEADER_SCANNER_HPP
#define PLYWOOT_HEADER_SCANNER_HPP

/// \file

#include "std.hpp"

#include <cstdint>
#include <cstring>
#include <istream>
#include <string>
#include <string_view>

namespace plywoot {

/// Base class for all header scanner exceptions.
struct HeaderScannerException : Exception
{
  /// Constructs a header scanner exception with the given exception message.
  ///
  /// \param message exception message
  HeaderScannerException(const std::string &message) : Exception("scanner error: " + message) {}
};

/// Exception thrown in case some stream in an invalid state was passed to the
/// header parser.
struct InvalidInputStream : HeaderScannerException
{
  /// Constructs an invalid input stream exception.
  InvalidInputStream() : HeaderScannerException("invalid input stream") {}
};

}

namespace plywoot::detail {

static constexpr const char endHeaderToken[] = "end_header";

/// Lookup table to check whether a character is a token delimiter.
/// The following characters are token delimiters:
///
///  1. \<space> (32)
///  2. \\t      (9)
///  3. \\n      (10)
///  4. \\r      (13)
///  5. EOF     (255)
///
// clang-format off
constexpr bool isTokenDelimiter[256] = {
  // 0      1      2      3      4      5      6      7      8      9     10     11     12     13     14     15
  false, false, false, false, false, false, false, false, false, true,  true,  false, false, true,  false, false, // 0   - 15
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, // 16  - 31
  true,  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, // 32  - 47
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, // 48  - 63
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, // 64  - 79
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, // 80  - 95
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, // 96  - 111
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, // 112 - 127
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, // 128 - 143
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, // 144 - 159
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, // 160 - 175
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, // 176 - 191
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, // 192 - 207
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, // 208 - 223
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, // 224 - 239
  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true , // 240 - 255
};
// clang-format on

/// Tokenizes the header data of an input PLY data stream. The scanner is
/// automatically constructed by the parser.
class HeaderScanner
{
public:
  /// Constructs a header scanner for the PLY header defined in the given input
  /// stream. Throws `InvalidInputStream` in case the input stream is not valid.
  HeaderScanner(std::istream &is) : is_{is}
  {
    if (!is) { throw InvalidInputStream{}; }

    std::string line;
    while (bool(std::getline(is, line)) && line != endHeaderToken)
    {
      buffer_.append(line);
      buffer_.push_back('\n');
    }

    if (line == endHeaderToken)
    {
      buffer_.append(line);
      buffer_.push_back('\n');
    }
    else { buffer_.push_back(EOF); }

    // Initialize the read head to the start of the buffered data.
    c_ = buffer_.data();
  }

  /// Enumeration of all PLY header token types.
  enum class Token {
    Unknown = 0,
    Ascii,
    BinaryBigEndian,
    BinaryLittleEndian,
    Char,
    Comment,
    Double,
    Element,
    EndHeader,
    Eof,
    Float,
    FloatingPointNumber,
    Format,
    Identifier,
    Int,
    List,
    MagicNumber,
    Number,
    Property,
    Short,
    UChar,
    UInt,
    UShort,
  };

  /// Returns the next token type in the input stream.
  Token nextToken() noexcept
  {
    // Ignore all whitespace, read upto the first non-whitespace character.
    const char *last = buffer_.data() + buffer_.size();
    while (c_ < last && 0 <= *c_ && *c_ <= 0x20)
    {
      if (*c_ == '\n') ++line_;
      ++c_;
    }

    // Read an identifier. After an identifier is read, the read head is
    // positioned at the start of the next token or whitespace.
    const char *tokenStart = c_;
    while (!isTokenDelimiter[(unsigned char)*c_]) { c_++; }
    tokenString_ = std::string_view(tokenStart, c_ - tokenStart);

    // In case the identifier is one of the reserved keywords, handle it as
    // such. Use first character for quick comparison.
    switch (tokenString_.front())
    {
      case 'a':
        token_ = !tokenString_.compare("ascii") ? Token::Ascii : Token::Identifier;
        break;
      case 'b':
        if (!tokenString_.compare("binary_big_endian"))
          token_ = Token::BinaryBigEndian;
        else if (!tokenString_.compare("binary_little_endian"))
          token_ = Token::BinaryLittleEndian;
        else
          token_ = Token::Identifier;
        break;
      case 'c':
        if (!tokenString_.compare("char")) { token_ = Token::Char; }
        else if (!tokenString_.compare("comment"))
        {
          token_ = Token::Comment;
          readComment();
        }
        else { token_ = Token::Identifier; }
        break;
      case 'd':
        token_ = (!tokenString_.compare("double") ? Token::Double : Token::Identifier);
        break;
      case 'e':
        if (!tokenString_.compare("element"))
          token_ = Token::Element;
        else if (!tokenString_.compare("end_header"))
          token_ = Token::EndHeader;
        else
          token_ = Token::Identifier;
        break;
      case 'f':
        if (!tokenString_.compare("format"))
          token_ = Token::Format;
        else if (!tokenString_.compare("float") || !tokenString_.compare("float32"))
          token_ = Token::Float;
        else if (!tokenString_.compare("float64"))
          token_ = Token::Double;
        else
          token_ = Token::Identifier;
        break;
      case 'l':
        token_ = (!tokenString_.compare("list") ? Token::List : Token::Identifier);
        break;
      case 'i':
        if (!tokenString_.compare("int") || !tokenString_.compare("int32"))
          token_ = Token::Int;
        else if (!tokenString_.compare("int8"))
          token_ = Token::Char;
        else if (!tokenString_.compare("int16"))
          token_ = Token::Short;
        else
          token_ = Token::Identifier;
        break;
      case 'p':
        if (!tokenString_.compare("ply"))
          token_ = Token::MagicNumber;
        else if (!tokenString_.compare("property"))
          token_ = Token::Property;
        else
          token_ = Token::Identifier;
        break;
      case 's':
        token_ = (!tokenString_.compare("short") ? Token::Short : Token::Identifier);
        break;
      case 'u':
        if (!tokenString_.compare("uint8") || !tokenString_.compare("uchar"))
          token_ = Token::UChar;
        else if (!tokenString_.compare("uint16") || !tokenString_.compare("ushort"))
          token_ = Token::UShort;
        else if (!tokenString_.compare("uint32") || !tokenString_.compare("uint"))
          token_ = Token::UInt;
        else
          token_ = Token::Identifier;
        break;
      case '-':
      case '+':
      case '.':
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        // TODO(ton): scientific notation for floating point numbers?
        token_ = (tokenString_.find('.') != std::string::npos) ? Token::FloatingPointNumber : Token::Number;
        break;
      case EOF:
        token_ = Token::Eof;
        break;
      default:
        token_ = Token::Identifier;
        break;
    }

    return token_;
  }

  /// Returns whether a token is a reserved keyword.
  static constexpr bool isKeyword(Token token)
  {
    switch (token)
    {
      case Token::Ascii:
      case Token::BinaryBigEndian:
      case Token::BinaryLittleEndian:
      case Token::Char:
      case Token::Double:
      case Token::Element:
      case Token::EndHeader:
      case Token::Float:
      case Token::Format:
      case Token::Int:
      case Token::List:
      case Token::Property:
      case Token::Short:
      case Token::UChar:
      case Token::UInt:
      case Token::UShort:
        return true;
      default:
        break;
    }

    return false;
  }

  /// In case the current token represents a comment, returns a comment
  /// instance, containing the line number the comment is defined on, and the
  /// comment text.
  Comment comment() const { return {line_, tokenString()}; }

  /// Returns the most recently scanned token.
  Token token() const noexcept { return token_; }

  /// Converts the current token string to a number of the given type.
  std::size_t tokenNumber() const noexcept
  {
    return static_cast<std::size_t>(std::strtoull(tokenString_.data(), nullptr, 10));
  }

  /// Returns the string representation of the current token.
  std::string tokenString() const noexcept { return std::string(tokenString_.data(), tokenString_.size()); }

private:
  /// Reads the remainder of the line as a comment. The comment itself is set
  /// as the token string.
  void readComment()
  {
    // Skip spaces and tabs and the first non-whitespace character.
    const char *end = buffer_.data() + buffer_.size();
    while (c_ < end && (*c_ == ' ' || *c_ == '\t')) { ++c_; }

    const std::size_t remainingBytes = buffer_.size() - (c_ - buffer_.data());
    const char *last = static_cast<const char *>(::memchr(c_, '\n', remainingBytes));
    if (last != nullptr)
    {
      tokenString_ = std::string_view(c_, last - c_);
      c_ = last;
    }
  }

  /// Buffered data, always a null terminated string.
  std::string buffer_;
  /// Character the scanner's read head is currently pointing to. Invariant
  /// after construction of the scanner is:
  ///
  ///       buffer_.data() <= c_ < (buffer_.data() + buffer_.size())
  ///
  const char *c_{buffer_.data()};

  /// Most recently scanned token.
  Token token_{Token::Unknown};
  /// String representation of the current token in case it is not predefined.
  std::string_view tokenString_;
  /// Current line number.
  std::uint32_t line_{0};

  /// Reference to the wrapped input stream.
  std::istream &is_;
};

}

#endif
