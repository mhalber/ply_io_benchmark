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

#ifndef PLYWOOT_ASCII_WRITER_POLICY_HPP
#define PLYWOOT_ASCII_WRITER_POLICY_HPP

/// \file

#include "buffered_ostream.hpp"

#include <ostream>

namespace plywoot::detail {

namespace {

template<typename T>
struct CharToInt
{
  template<typename U>
  U operator()(U &&u) const
  {
    return u;
  }

  int operator()(char c) const { return static_cast<int>(c); }
  int operator()(signed char c) const { return static_cast<int>(c); }
  unsigned operator()(unsigned char c) const { return static_cast<unsigned>(c); }
};

}

/// Defines a writer policy that deals with ASCII output streams.
class AsciiWriterPolicy
{
public:
  AsciiWriterPolicy(std::ostream &os) : os_{os} {}

  /// Writes the number `t` of the given type `T` to the given ASCII output
  /// stream.
  template<typename T>
  void writeNumber(T t) const
  {
    os_.writeAscii(CharToInt<T>{}(t));
  }

  /// Writes a list of numbers of type `SrcT` to the given ASCII output stream,
  /// which includes the size of that list.
  /// The first template argument represents the type of the size of the list in
  /// the output PLY file, which can be ignored for ASCII PLY formats, as is the
  /// type of the PLY number type.
  template<typename PlySizeT, typename PlyT, typename SrcT>
  void writeList(const SrcT *t, std::size_t n) const
  {
    os_.writeAscii(n);
    if (n > 0)
    {
      os_.put(' ');
      writeNumbers<PlyT, SrcT>(t, n);
    }
  }

  /// Writes a list of numbers of type `SrcT` to the given ASCII output stream.
  /// The first template argument represents the type of the size of the list in
  /// the output PLY file, which can be ignored for ASCII PLY formats, as is the
  /// type of the PLY number type.
  template<typename PlyT, typename SrcT>
  void writeNumbers(const SrcT *t, std::size_t n) const
  {
    for (std::size_t i = 0; i < n - 1; ++i)
    {
      os_.writeAscii(*t++);
      os_.put(' ');
    }
    os_.writeAscii(*t);
  }

  /// Outputs empty data for the range of properties [`first`, `last`). Note
  /// that a property that is undefined is always stored as a zero character in
  /// ASCII mode; regardless whether the property is a list of a single element,
  /// since in case of a list we store a zero-element list.
  void writeMissingProperties(PlyPropertyConstIterator first, PlyPropertyConstIterator last) const
  {
    while (first++ != last) { os_.write(" 0", 2); }
  }

  /// Writes a newline separator.
  void writeNewline() const { os_.put('\n'); }

  /// Writes a token separator (a space).
  void writeTokenSeparator() const { os_.put(' '); }

private:
  mutable detail::BufferedOStream os_;
};

}

#endif
