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

#ifndef PLYWOOT_ENDIAN_HPP
#define PLYWOOT_ENDIAN_HPP

/// \file

#include <cstdint>
#include <utility>

#ifndef _WIN32
#include <endian.h>
#endif


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

/// Naive byte swap functions for floating point numbers to convert between
/// endian representations.
///
/// \param t floating point number to convert
/// \return \p t converted from either big/little to little/big endian
template<typename T>
T byte_swap(T t)
{
  unsigned char *bytes = reinterpret_cast<unsigned char *>(&t);

  if constexpr (sizeof(T) == 2)
  {
    std::swap(bytes[0], bytes[1]);
  }
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

/// Wrapper around the glibc htobe* functions that pick the correct call
/// depending on the size of the argument type, converts the number object from
/// host endianess to big endian.
///
/// \param t number to convert
/// \return \p t in big endian representation
template<typename T>
T htobe(T t)
{
#ifndef _WIN32
  if constexpr (sizeof(T) == 1) { return t; }
  else if constexpr (std::is_integral_v<T> && sizeof(T) == 2) { return static_cast<T>(htobe16(t)); }
  else if constexpr (std::is_integral_v<T> && sizeof(T) == 4) { return static_cast<T>(htobe32(t)); }
  else if constexpr (std::is_integral_v<T> && sizeof(T) == 8) { return static_cast<T>(htobe64(t)); }
  else if constexpr (std::is_floating_point_v<T>) { return byte_swap(t); }
#else
  return byte_swap(t);
#endif
}

/// Wrappers around the glibc be*toh functions that pick the correct call
/// depending on the size of the argument type, converts the number object from
/// big endian to host endianess.
///
/// \param t big endian number to convert
/// \return \p t in host endian representation
template<typename T>
T betoh(T t)
{
#ifndef _WIN32
  if constexpr (sizeof(T) == 1) { return t; }
  else if constexpr (std::is_integral_v<T> && sizeof(T) == 2) { return static_cast<T>(be16toh(t)); }
  else if constexpr (std::is_integral_v<T> && sizeof(T) == 4) { return static_cast<T>(be32toh(t)); }
  else if constexpr (std::is_integral_v<T> && sizeof(T) == 8) { return static_cast<T>(be64toh(t)); }
  else if constexpr (std::is_floating_point_v<T>) { return byte_swap(t); }
#else
  return byte_swap(t);
#endif
}

}

#endif
