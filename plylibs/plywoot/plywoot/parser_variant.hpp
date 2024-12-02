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

#ifndef PLYWOOT_PARSER_VARIANT_HPP
#define PLYWOOT_PARSER_VARIANT_HPP

/// \file

#include "ascii_parser_policy.hpp"
#include "binary_parser_policy.hpp"
#include "parser.hpp"

#include <istream>
#include <variant>

namespace plywoot::detail {

/// For lack of `std::variant`, create a cheap tagged union.
class ParserVariant
{
public:
  ParserVariant(std::istream &is, PlyFormat format) : variant_{makeVariant(is, format)} {}

  ParserVariant(const ParserVariant &) = delete;
  ParserVariant &operator=(const ParserVariant &) = delete;

  PlyElementData read(const PlyElement &element) const
  {
    return std::visit([&](auto &&parser) { return parser.read(element); }, variant_);
  }

  template<typename... Ts>
  void read(const PlyElement &element, reflect::Layout<Ts...> layout) const
  {
    std::visit([&element, layout](auto &&parser) { parser.read(element, layout); }, variant_);
  }

  void skip(const PlyElement &element) const
  {
    std::visit([&element](auto &&parser) { parser.skip(element); }, variant_);
  }

private:
  using Variant = std::variant<
      detail::Parser<detail::AsciiParserPolicy>,
      detail::Parser<detail::BinaryBigEndianParserPolicy>,
      detail::Parser<detail::BinaryLittleEndianParserPolicy>>;

  Variant makeVariant(std::istream &is, PlyFormat format)
  {
    switch (format)
    {
      case PlyFormat::Ascii:
        return Variant{std::in_place_type<detail::Parser<detail::AsciiParserPolicy>>, is};
      case PlyFormat::BinaryBigEndian:
        return Variant{std::in_place_type<detail::Parser<detail::BinaryBigEndianParserPolicy>>, is};
      case PlyFormat::BinaryLittleEndian:
      default:
        break;
    }

    return Variant{std::in_place_type<detail::Parser<detail::BinaryLittleEndianParserPolicy>>, is};
  }

  Variant variant_;
};

}

#endif
