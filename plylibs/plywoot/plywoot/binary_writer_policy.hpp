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

#ifndef PLYWOOT_BINARY_WRITER_POLICY_HPP
#define PLYWOOT_BINARY_WRITER_POLICY_HPP

/// \file

#include "buffered_ostream.hpp"
#include "endian.hpp"

#include <ostream>

namespace plywoot::detail {

/// Defines a writer policy that deals with binary output streams.
template<typename Endianness>
class BinaryWriterPolicy
{
public:
  BinaryWriterPolicy(std::ostream &os) : os_{os} {}

  /// Writes the number `t` of the given type `T` to the given binary output
  /// stream in little endian format.
  template<typename T, typename EndiannessDependent = Endianness>
  void writeNumber(T t) const
  {
    // TODO(ton): this assumes the target architecture is little endian.
    if constexpr (std::is_same_v<EndiannessDependent, LittleEndian>)
    {
      os_.write(reinterpret_cast<const char *>(&t), sizeof(T));
    }
    else
    {
      const auto be = htobe(t);
      os_.write(reinterpret_cast<const char *>(&be), sizeof(T));
    }
  }

  /// Writes a list of numbers of type `PlyT` to the given binary output stream,
  /// either in little or big endian format, reading `n` numbers starting at
  /// address `src`, of number type `SrcT`. This also writes the size of the
  /// list to the output stream using number type `PlySizeT`.
  template<typename PlySizeT, typename PlyT, typename SrcT, typename EndiannessDependent = Endianness>
  void writeList(const SrcT *t, std::size_t n) const
  {
    writeNumber<PlySizeT>(n);
    writeNumbers<PlyT, SrcT>(t, n);
  }

  /// Writes `n` numbers of type `SrcT` to the given binary output stream, as
  /// numbers of type `PlyT`, either in little or big endian format.
  template<typename PlyT, typename SrcT, typename EndiannessDependent = Endianness>
  void writeNumbers(const SrcT *t, std::size_t n) const
  {
    if constexpr (std::is_same_v<PlyT, SrcT> && std::is_same_v<EndiannessDependent, LittleEndian>)
    {
      os_.write(reinterpret_cast<const char *>(t), sizeof(SrcT) * n);
    }
    else
    {
      for (std::size_t i = 0; i < n; ++i) { writeNumber<PlyT>(*t++); }
    }
  }

  /// Outputs empty data for the range of properties [`first`, `last`). Note
  /// that a property that is undefined is always stored as a zero number, where
  /// the type of the number depends on the underlying property; in case of a
  /// list property the size type determines the number type, otherwise the
  /// regular property type is used.
  void writeMissingProperties(PlyPropertyConstIterator first, PlyPropertyConstIterator last) const
  {
    while (first != last)
    {
      switch (first->isList() ? first->sizeType() : first->type())
      {
        case PlyDataType::Char:
          writeNumber<char>(0);
          break;
        case PlyDataType::UChar:
          writeNumber<unsigned char>(0);
          break;
        case PlyDataType::Short:
          writeNumber<short>(0);
          break;
        case PlyDataType::UShort:
          writeNumber<unsigned short>(0);
          break;
        case PlyDataType::Int:
          writeNumber<int>(0);
          break;
        case PlyDataType::UInt:
          writeNumber<unsigned int>(0);
          break;
        case PlyDataType::Float:
          writeNumber<float>(0);
          break;
        case PlyDataType::Double:
          writeNumber<double>(0);
          break;
      }
      ++first;
    }
  }

  /// Writes a newline, ignored for binary output formats.
  void writeNewline() const {}
  /// Writes a token separator, ignored for binary output formats.
  void writeTokenSeparator() const {}

private:
  mutable detail::BufferedOStream os_;
};

/// Convenience type alias for the binary little endian writer policy.
using BinaryLittleEndianWriterPolicy = BinaryWriterPolicy<LittleEndian>;
/// Convenience type alias for the binary big endian writer policy.
using BinaryBigEndianWriterPolicy = BinaryWriterPolicy<BigEndian>;

}

#endif
