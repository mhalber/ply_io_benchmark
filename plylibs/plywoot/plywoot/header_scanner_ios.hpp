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

#ifndef PLYWOOT_HEADER_SCANNER_IOS_HPP
#define PLYWOOT_HEADER_SCANNER_IOS_HPP

/// \file

#include <ostream>

namespace plywoot::detail {

/// Outputs the text representation of the given header scanner token \p t to
/// the given output stream \p os.
///
/// \param os output stream to output the text representation of \p t to
/// \param t header scanner token to convert to text
/// \return the output stream
inline std::ostream &operator<<(std::ostream &os, HeaderScanner::Token t)
{
  switch (t)
  {
    case HeaderScanner::Token::Unknown:
      return os << "<unknown>";
    case HeaderScanner::Token::Ascii:
      return os << "ascii";
    case HeaderScanner::Token::BinaryBigEndian:
      return os << "binary_big_endian";
    case HeaderScanner::Token::BinaryLittleEndian:
      return os << "binary_little_endian";
    case HeaderScanner::Token::Char:
      return os << "char";
    case HeaderScanner::Token::Comment:
      return os << "comment";
    case HeaderScanner::Token::Double:
      return os << "double";
    case HeaderScanner::Token::Element:
      return os << "element";
    case HeaderScanner::Token::EndHeader:
      return os << "end_header";
    case HeaderScanner::Token::Eof:
      return os << "<eof>";
    case HeaderScanner::Token::Float:
      return os << "float";
    case HeaderScanner::Token::FloatingPointNumber:
      return os << "<floating point number>";
    case HeaderScanner::Token::Format:
      return os << "format";
    case HeaderScanner::Token::Identifier:
      return os << "<identifier>";
    case HeaderScanner::Token::Int:
      return os << "int";
    case HeaderScanner::Token::List:
      return os << "list";
    case HeaderScanner::Token::Number:
      return os << "<number>";
    case HeaderScanner::Token::MagicNumber:
      return os << "ply";
    case HeaderScanner::Token::Property:
      return os << "property";
    case HeaderScanner::Token::Short:
      return os << "short";
    case HeaderScanner::Token::UChar:
      return os << "uchar";
    case HeaderScanner::Token::UInt:
      return os << "uint";
    case HeaderScanner::Token::UShort:
      return os << "ushort";
  }

  return os;
}

}

#endif
