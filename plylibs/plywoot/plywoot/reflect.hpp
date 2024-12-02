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

#ifndef PLYWOOT_REFLECT_HPP
#define PLYWOOT_REFLECT_HPP

/// \file

#include <type_traits>
#include <vector>

namespace plywoot::reflect {

/// Type wrapper that wraps some type (`DestT`) that we need to serialize to. A
/// specialization exists that extracts the destination type from some of the
/// reflect helper types.
/// @{
template<typename T, typename = void>
struct Type
{
  /// Target property type.
  using DestT = T;
};

template<typename T>
struct Type<T, typename std::void_t<typename T::DestT>>
{
  /// Target property type.
  using DestT = typename T::DestT;
};
/// @}

/// Can be embedded in a `Layout` type to read an element list property of some
/// fixed size `N`, with elements of type `T`. The size type is used in the PLY
/// format to store the type of the length of the list.
template<typename T, std::size_t N>
struct Array
{
  /// Type of the elements in the destination list type.
  using DestT = T;
};

/// Can be embedded in a `Layout` type to skip an element property in the input
/// PLY file (only useful when reading data from a PLY stream).
struct Skip
{
};

/// Can be used in a `Layout` type to step over member variables in the
/// destination structure (only useful when reading data from a PLY stream).
template<typename T>
struct Stride
{
};

/// Can be used in a `Layout` type to pack together a sequence of properties of
/// the same type, such that they will be parsed in one go, speeding up parsing.
template<typename T, std::size_t N>
struct Pack
{
  /// Type of the member types in the target type to pack.
  using DestT = T;
  /// Number of PLY properties to pack together and `memcpy` to the
  /// corresponding target type members.
  static constexpr std::size_t size = N;
};

/// Used to define the layout of some structure that is either read from or
/// written to by the PLY I/O functions. Note that it is important that the
/// order of data types specified in the layout matches the order of the data
/// types in the data type that is read from or written to. This will
/// automatically take default C/C++ padding into account. In case not all
/// properties in some layout structure are written, use `reflect::Stride` to
/// skip properties. Properties at the end of the structure that are not read
/// from or written do not need to be specified and will automatically be
/// skipped.
template<typename... Ts>
class Layout
{
public:
  /// Constructor for an empty layout (no data will be read/written).
  Layout() = default;

  /// Constructs a layout representation of some element, and specifies a target
  /// list of elements that will be written to by the PLY parser.
  ///
  /// \param v list of elements to be assigned to by the PLY parser
  template<typename T>
  Layout(std::vector<T> &v)
      : data_{reinterpret_cast<std::uint8_t *>(v.data())},
        cdata_{data_},
        size_{v.size()},
        alignment_{alignof(T)}
  {
  }

  /// Constructs a layout representation of some element, and specifies a target
  /// list of elements that will be read from by the PLY writer.
  ///
  /// \param v list of elements to be read from by the PLY writer
  template<typename T>
  Layout(const std::vector<T> &v)
      : data_{nullptr},
        cdata_{reinterpret_cast<const std::uint8_t *>(v.data())},
        size_{v.size()},
        alignment_{alignof(T)}
  {
  }

  /// Returns a pointer to the memory area storing instances of type `T`
  /// associated with this layout.
  ///
  /// \return a pointer to the memory area storing instances of type `T`
  ///    associated with this layout.
  std::uint8_t *data() { return data_; }
  /// Returns a pointer to the read-only memory area storing instances of type
  /// `T` associated with this layout.
  ///
  /// \return a pointer to the read-only memory area storing instances of type
  ///    `T` associated with this layout.
  const std::uint8_t *data() const { return cdata_; }

  /// Returns the number of elements that are or may be stored in the memory
  /// block pointer to by the associated data block.
  ///
  /// \return the number of elements that are or may be stored in the memory
  ///     block pointer to by the associated data block
  std::size_t size() const { return size_; }

  /// Alignment requirements of this structure.
  ///
  /// \return alignment requirements of the elements of type T in this layout
  std::size_t alignment() const { return alignment_; }

private:
  /// Pointer to the writable memory area that contains `n` number of structures
  /// made up of the types associated with this layout.
  std::uint8_t *data_{nullptr};
  /// Pointer to the read-only memory area that contains `n` number of
  /// structures made up of the types associated with this layout.
  const std::uint8_t *cdata_{nullptr};
  /// Number of elements that are or may be stored in the memory block pointer
  /// to by the associated data block.
  std::size_t size_{0};
  /// Alignment requirements of the structure instances stored in this memory
  /// block.
  std::size_t alignment_{0};
};

}

#endif
