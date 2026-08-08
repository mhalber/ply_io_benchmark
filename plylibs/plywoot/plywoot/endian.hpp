/*
   This file is part of PLYwoot, a header-only PLY parser.

   Copyright (C) 2023-2026, Ton van den Heuvel

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

#ifndef PLYWOOT_ENDIAN_HPP
#define PLYWOOT_ENDIAN_HPP

/// \file

#include <type_traits>
#include <utility>

namespace plywoot::detail {

/// Type tag to indicate little endian behavior is required for the
/// parser/writer policies.
struct LittleEndian
{
};

/// Type tag to indicate big endian behavior is required for the parser/writer
/// policies.
struct BigEndian
{
};

/// Alias type encoding the host platform endianness. Based on the possible
/// implementation for `std::endian` documented on cppreference.com.
#if defined(_MSC_VER) && !defined(__clang__)
using HostEndian = LittleEndian;
#else
using HostEndian = std::conditional_t<__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__, LittleEndian, BigEndian>;
#endif

/// Naive byte swap functions for floating point numbers to convert between
/// endian representations.
///
/// \param t floating point number to convert
/// \return \p t converted from either big/little to little/big endian
template<typename T>
T byte_swap(T t)
{
  unsigned char *bytes = reinterpret_cast<unsigned char *>(&t);

  if constexpr (sizeof(T) == 2) { std::swap(bytes[0], bytes[1]); }
  else if constexpr (sizeof(T) == 4)
  {
    std::swap(bytes[0], bytes[3]);
    std::swap(bytes[1], bytes[2]);
  }
  else if constexpr (sizeof(T) == 8)
  {
    std::swap(bytes[0], bytes[7]);
    std::swap(bytes[1], bytes[6]);
    std::swap(bytes[2], bytes[5]);
    std::swap(bytes[3], bytes[4]);
  }

  return t;
}

}

#endif
