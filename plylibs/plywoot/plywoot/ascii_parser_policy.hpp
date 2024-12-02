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

#ifndef PLYWOOT_ASCII_PARSER_POLICY_HPP
#define PLYWOOT_ASCII_PARSER_POLICY_HPP

/// \file

#include "buffered_istream.hpp"
#include "exceptions.hpp"
#include "reflect.hpp"
#include "std.hpp"
#include "types.hpp"

#include <cstdint>
#include <string>

namespace plywoot {

/// Base class for all parser exceptions.
struct ParserException : Exception
{
  /// Constructs a parser exception with the given exception message.
  ///
  /// \param message parser exception message
  ParserException(const std::string &message) : Exception("parser error: " + message) {}
};

/// Unexpected end-of-file exception.
struct UnexpectedEof : ParserException
{
  /// Constructs an unexpected end-of-file exception.
  UnexpectedEof() : ParserException("unexpected end of file") {}
};

namespace detail {

/// Defines a parser policy that deals with ASCII input streams. The policy
/// types act as mixins to add format specific behavior to the generic parser
/// functionality in `plywoot::detail::Parser`.
class AsciiParserPolicy
{
public:
  /// Constructs an ASCII parser policy instance for the given input stream.
  ///
  /// \param is input stream to associate with this ASCII input stream policy.
  AsciiParserPolicy(std::istream &is) : is_{is} {}

  /// Skips the given element in the current input stream, assuming the read
  /// head is at the start of that element.
  ///
  /// \param e element to skip
  void skipElement(const PlyElement &e) const { is_.skipLines(e.size()); }

  /// Skips the given property in the current input stream.
  ///
  /// \param p property to skip
  void skipProperty(const PlyProperty &p) const
  {
    if (p.isList())
    {
      // TODO(ton): maybe switch on the list size type...?
      const auto size = readNumber<int>();
      for (int i = 0; i < size; ++i) { skipNumber(); }
    }
    else { skipNumber(); }
  }

  /// Reads a number of the given type `T` from the input stream.
  ///
  /// \return a number of the given type `T` read from the input stream
  template<typename T>
  T readNumber() const
  {
    is_.skipWhitespace();
    if (is_.eof()) { throw UnexpectedEof(); }

    is_.buffer(256);
    return detail::to_number<T>(is_.data(), is_.data() + 256, &is_.data());
  }

  /// Reads `N` numbers of the given type `PlyT` from the input stream, and
  /// stores them contiguously at the given destination in memory as numbers of
  /// type `DestT`.
  ///
  /// \param dest pointer to the destination in memory where to store parsed
  ///     numbers
  /// \return a pointer pointing just after the last number stored \p dest
  template<typename PlyT, typename DestT, std::size_t N>
  std::uint8_t *readNumbers(std::uint8_t *dest) const
  {
    // TODO(ton): needs to be specialized for improved performance.
    for (std::size_t i = 0; i < N; ++i, dest += sizeof(DestT))
    {
      *reinterpret_cast<DestT *>(dest) = readNumber<PlyT>();
    }
    return dest;
  }

  /// Skips a number of the given type `T` in the input stream.
  template<typename T = void>
  void skipNumber() const
  {
    is_.skipWhitespace();
    is_.skipNonWhitespace();
  }

  /// Skips property data, totaling \p n bytes.
  ///
  /// \param n number of bytes to skip
  void skipProperties(std::size_t n) const
  {
    if (n > 0) is_.skipLines(1);
  }

private:
  /// Wrapped input stream associated with this ASCII parser policy.
  mutable detail::BufferedIStream is_;
};

}
}

#endif
