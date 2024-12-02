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

#ifndef PLYWOOT_BINARY_PARSER_POLICY_HPP
#define PLYWOOT_BINARY_PARSER_POLICY_HPP

/// \file

#include "buffered_istream.hpp"
#include "endian.hpp"
#include "type_traits.hpp"

#include <cstdint>
#include <numeric>

namespace plywoot::detail {

/// Defines a parser policy that deals with binary input streams. The policy
/// types act as mixins to add format specific behavior to the generic parser
/// functionality in `plywoot::detail::Parser`.
template<typename Endianness>
class BinaryParserPolicy
{
public:
  /// Constructs a binary little endian parser policy.
  ///
  /// \param is input stream to associate with this binary input stream policy.
  BinaryParserPolicy(std::istream &is) : is_{is} {}

  /// Skips the given element in the current input stream, assuming the read
  /// head is at the start of that element.
  ///
  /// \param e element to skip
  void skipElement(const PlyElement &e) const
  {
    // In case all element properties are non-list properties, we can calculate
    // the element size in one go.
    const std::vector<PlyProperty> &properties = e.properties();
    if (std::all_of(properties.begin(), properties.end(), [](const PlyProperty &p) { return !p.isList(); }))
    {
      is_.skip(
          e.size() * std::accumulate(
                         properties.begin(), properties.end(), 0,
                         [](std::size_t acc, const PlyProperty &p) { return acc + sizeOf(p.type()); }));
    }
    else
    {
      for (std::size_t i = 0; i < e.size(); ++i)
      {
        for (const PlyProperty &p : e.properties()) { skipProperty(p); }
      }
    }
  }

  /// Skips the given property in the current input stream, assuming the read
  /// head is at the start of that element.
  ///
  /// \param p property to skip
  void skipProperty(const PlyProperty &p) const
  {
    if (!p.isList()) { is_.skip(sizeOf(p.type())); }
    else
    {
      std::size_t size = 0;
      switch (p.sizeType())
      {
        case PlyDataType::Char:
          size = readNumber<char>();
          break;
        case PlyDataType::UChar:
          size = readNumber<unsigned char>();
          break;
        case PlyDataType::Short:
          size = readNumber<short>();
          break;
        case PlyDataType::UShort:
          size = readNumber<unsigned short>();
          break;
        case PlyDataType::Int:
          size = readNumber<int>();
          break;
        case PlyDataType::UInt:
          size = readNumber<unsigned int>();
          break;
        case PlyDataType::Float:
          size = readNumber<float>();
          break;
        case PlyDataType::Double:
          size = readNumber<double>();
          break;
      }

      is_.skip(size * sizeOf(p.type()));
    }
  }

  /// Reads a number of the given type `T` from the input stream.
  ///
  /// \return a number of the given type `T` read from the input stream
  /// @{
  template<typename T, typename EndiannessDependent = Endianness>
  T readNumber() const
  {
    // TODO(ton): this assumes the target architecture is little endian.
    if constexpr (std::is_same_v<EndiannessDependent, LittleEndian>) { return is_.read<T>(); }
    else { return betoh(is_.read<T>()); }
  }

  /// Reads `N` numbers of the given type `PlyT` from the input stream, and
  /// stores them contiguously at the given destination in memory as numbers of
  /// type `DestT`. Returns a pointer pointing just after the last number stored
  /// at `dest`.
  ///
  /// \param dest pointer to the destination in memory where to store parsed
  ///     numbers
  /// \return a pointer pointing just after the last number stored \p dest
  template<typename PlyT, typename DestT, std::size_t N, typename EndiannessDependent = Endianness>
  std::uint8_t *readNumbers(std::uint8_t *dest) const
  {
    if constexpr (std::is_same_v<EndiannessDependent, LittleEndian>)
    {
      return is_.read<PlyT, DestT, N>(dest);
    }
    else
    {
      std::uint8_t *result = is_.read<PlyT, DestT, N>(dest);

      // Perform endianess conversion.
      DestT *to = reinterpret_cast<DestT *>(dest);
      for (std::size_t i = 0; i < N; ++i, ++to) { *to = betoh(static_cast<PlyT>(*to)); }

      return result;
    }
  }

  /// Skips a number of the given type `T` in the input stream.
  template<typename T>
  void skipNumber() const
  {
    is_.skip(sizeof(T));
  }

  /// Skips property data, totaling \p n bytes.
  ///
  /// \param n number of bytes to skip
  void skipProperties(std::size_t n) const { is_.skip(n); }

  /// Copies all element data to the given destination buffer `dest`. This
  /// assumes an element maps to a collection of types `Ts...` for which all
  /// types are trivially copyable, and contiguous in memory without any padding
  /// in between.
  ///
  /// \param dest
  /// \param element
  template<typename... Ts, typename EndiannessDependent = Endianness>
  void memcpy(std::uint8_t *dest, const PlyElement &element) const
  {
    is_.memcpy(dest, element.size() * detail::sizeOf<Ts...>());

    if constexpr (std::is_same_v<EndiannessDependent, BigEndian>)
    {
      for (std::size_t i = 0; i < element.size(); ++i) { dest = toBigEndian<Ts...>(dest); }
    }
  }

private:
  template<typename T>
  std::uint8_t *toBigEndian(std::uint8_t *dest) const
  {
    if constexpr (!detail::IsPack<T>::value)
    {
      auto ptr = reinterpret_cast<T *>(dest);
      *ptr = htobe(*ptr);

      return dest + sizeof(T);
    }
    else
    {
      auto ptr = reinterpret_cast<typename T::DestT *>(dest);
      for (std::size_t i = 0; i < T::size; ++i, ++ptr) { *ptr = htobe(*ptr); }

      return dest + detail::sizeOf<T>();
    }
  }

  template<typename T, typename U, typename... Ts>
  std::uint8_t *toBigEndian(std::uint8_t *dest) const
  {
    static_assert(
        detail::isPacked<T, U, Ts...>(),
        "converting possibly padded range of little-endian objects to big-endian not implemented yet");

    return toBigEndian<U, Ts...>(toBigEndian<T>(dest));
  }

  /// Wrapped input stream associated with this binary parser policy.
  mutable detail::BufferedIStream is_;
};

/// Convenience type alias for the binary little endian parser policy.
using BinaryLittleEndianParserPolicy = BinaryParserPolicy<LittleEndian>;
/// Convenience type alias for the binary big endian parser policy.
using BinaryBigEndianParserPolicy = BinaryParserPolicy<BigEndian>;

}

#endif
