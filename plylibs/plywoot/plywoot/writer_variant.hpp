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

#ifndef PLYWOOT_WRITER_VARIANT_HPP
#define PLYWOOT_WRITER_VARIANT_HPP

/// \file

#include "ascii_writer_policy.hpp"
#include "binary_writer_policy.hpp"
#include "writer.hpp"

#include <ostream>
#include <variant>

namespace plywoot::detail {

class WriterVariant
{
public:
  WriterVariant(std::ostream &os, PlyFormat format) : variant_{makeVariant(os, format)} {}

  WriterVariant(const WriterVariant &) = delete;
  WriterVariant &operator=(const WriterVariant &) = delete;

  void write(const PlyElement &element, const std::uint8_t *src, std::size_t alignment) const
  {
    std::visit(
        [&element, src, alignment](auto &&writer) { writer.write(element, src, alignment); }, variant_);
  }

  template<typename... Ts>
  void write(const PlyElement &element, const reflect::Layout<Ts...> layout) const
  {
    std::visit([&element, layout](auto &&writer) { writer.write(element, layout); }, variant_);
  }

private:
  using Variant = std::variant<
      detail::Writer<detail::AsciiWriterPolicy>,
      detail::Writer<detail::BinaryBigEndianWriterPolicy>,
      detail::Writer<detail::BinaryLittleEndianWriterPolicy>>;

  Variant makeVariant(std::ostream &os, PlyFormat format)
  {
    switch (format)
    {
      case PlyFormat::Ascii:
        return Variant{std::in_place_type<detail::Writer<detail::AsciiWriterPolicy>>, os};
      case PlyFormat::BinaryBigEndian:
        return Variant{std::in_place_type<detail::Writer<detail::BinaryBigEndianWriterPolicy>>, os};
      case PlyFormat::BinaryLittleEndian:
      default:
        break;
    }

    return Variant{std::in_place_type<detail::Writer<detail::BinaryLittleEndianWriterPolicy>>, os};
  }

  Variant variant_;
};

}

#endif
