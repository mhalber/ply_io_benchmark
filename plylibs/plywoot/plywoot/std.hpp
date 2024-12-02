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

#ifndef PLYWOOT_STD_HPP
#define PLYWOOT_STD_HPP

/// \file

#ifdef PLYWOOT_USE_FAST_FLOAT
#include <fast_float/fast_float.h>
#endif

#ifdef PLYWOOT_USE_FAST_INT
#include <fast_int/fast_int.hpp>
#endif

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <sstream>
#include <string>

namespace plywoot::detail {

/// Aligns the given input pointer. Implementation is taken from GCCs
/// `std::align` implementation. The given alignment value should be a power
/// of two.
///
/// \param ptr pointer to align
/// \param alignment alignment requirements of the given pointer
/// \return \p ptr aligned according to the given alignment requirements
template<typename Ptr>
constexpr Ptr align(Ptr ptr, std::size_t alignment)
{
  // Some explanation on the code below; -x is x in two's complement, which
  // means that an alignment value x of power two is converted to (~x + 1).
  // For example, for an alignment value of 4, this turns 0b000100 into
  // 0b111100. The factor (uintptr + alignment - 1u) guarantees that the
  // alignment bit is set unless (uintptr % alignment == 0).
  const auto uintptr = reinterpret_cast<uintptr_t>(ptr);
  return reinterpret_cast<Ptr>((uintptr + alignment - 1u) & -alignment);
}


/// Converts a text to an integer number. The <a
/// href="https://github.com/ton/fast_int">fast_int</a> library will be used to
/// perform the conversion in case it was found during the configuration stage.
///
/// \tparam Number integer number type
/// \param first start of the text range
/// \param last end of the text range
/// \param end position in the text range that denotes the end of the number
///     text
/// \return converted integer number
template<typename Number>
inline Number to_number(const char *first, const char *last, const char **end)
{
#ifdef PLYWOOT_USE_FAST_INT
  Number n{};
  *end = fast_int::from_chars(first, last, n).ptr;
  return n;
#else
  return std::strtoll(first, const_cast<char **>(end), 10);
#endif
}

/// Converts a text to a single precision floating point number. The <a
/// href="https://github.com/fastfloat/fast_float">fast_float</a> library will
/// be used to perform the conversion in case it was found during the
/// configuration stage.
///
/// \param first start of the text range
/// \param last end of the text range
/// \param end position in the text range that denotes the end of the number
///     text
/// \return converted single precision floating point number
template<>
inline float to_number<>(const char *first, const char *last, const char **end)
{
#ifdef PLYWOOT_USE_FAST_FLOAT
  float x;
  *end = fast_float::from_chars(first, last, x).ptr;
  return x;
#else
  return std::strtof(first, const_cast<char **>(end));
#endif
}

/// Converts a text to a double precision floating point number. The <a
/// href="https://github.com/fastfloat/fast_float">fast_float</a> library will
/// be used to perform the conversion in case it was found during the
/// configuration stage.
///
/// \param first start of the text range
/// \param last end of the text range
/// \param end position in the text range that denotes the end of the number
///     text
/// \return converted double precision floating point number
template<>
inline double to_number<double>(const char *first, const char *last, const char **end)
{
#ifdef PLYWOOT_USE_FAST_FLOAT
  double x;
  *end = fast_float::from_chars(first, last, x).ptr;
  return x;
#else
  return std::strtof(first, const_cast<char **>(end));
#endif
}

/// Returns whether the given string starts with the given prefix.
///
/// \param s string to check for the given \p prefix
/// \param prefix prefix text to find
/// \return \c true in case the given string starts with the given prefix, \c
///     false otherwise
inline bool starts_with(const std::string &s, const char *prefix) { return s.rfind(prefix, 0) == 0; }

/// Generic type to string conversion functionality, requiring that an
/// `std::ostream` << operator is defined for the type. This should only be used
/// in contexts where performance does not matter.
///
/// \param t object to convert to string
/// \return \p t converted to string
template<typename T>
std::string to_string(const T &t)
{
  std::stringstream oss;
  oss << t;
  return oss.str();
}

}

#endif
