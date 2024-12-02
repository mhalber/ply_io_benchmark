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

#ifndef PLYWOOT_TYPE_TRAITS_HPP
#define PLYWOOT_TYPE_TRAITS_HPP

/// \file

#include "reflect.hpp"
#include "std.hpp"
#include "types.hpp"

#include <algorithm>
#include <type_traits>

namespace plywoot::detail {

/// Returns whether the given type `T` is considered to be a list.
/// @{
template<typename T>
struct IsList
{
  static constexpr bool value = false;
};

template<typename T, std::size_t N>
struct IsList<reflect::Array<T, N>>
{
  static constexpr bool value = true;
};

template<typename T>
struct IsList<std::vector<T>>
{
  static constexpr bool value = true;
};

template<typename T>
struct IsList<reflect::Type<T>>
{
  static constexpr bool value = IsList<T>::value;
};
/// @}

/// Returns whether the given reflection type `T` represents a list.
///
/// \tparam T type for which to calculate whether it represents a list
/// \return \c true in case the given type `T` represents a list
template<typename T>
constexpr bool isList()
{
  return IsList<T>::value;
}

/// Given a reflect type, stores the number of properties spanned by the
/// reflection type. By default, every reflection type spans one property,
/// except for `plywoot::reflect::Pack`, which spans multiple properties by
/// definition.
template<typename... Ts>
struct NumProperties
{
  static constexpr std::size_t size = 0;
};

template<typename T, typename... Ts>
struct NumProperties<T, Ts...>
{
  static constexpr std::size_t size = NumProperties<T>::size + NumProperties<Ts...>::size;
};

template<typename T, std::size_t N>
struct NumProperties<reflect::Array<T, N>>
{
  static constexpr std::size_t size = 1;
};

template<typename T, std::size_t N>
struct NumProperties<reflect::Pack<T, N>>
{
  static constexpr std::size_t size = N;
};

template<typename T>
struct NumProperties<reflect::Stride<T>>
{
  static constexpr std::size_t size = 0;
};

template<typename T>
struct NumProperties<T>
{
  static constexpr std::size_t size = 1;
};

/// Returns the number of properties spanned by the given list of reflection
/// types. By default, every reflection type spans one property, except for
/// `plywoot::reflect::Pack`, which spans multiple properties by definition.
///
/// \return the number of properties spanned by the given list of reflection
///     types
template<typename... Ts>
constexpr std::size_t numProperties()
{
  return NumProperties<Ts...>::size;
}

/// Returns whether an object of type `T` represents that same object as an
/// object of the given PLY data type `type`.
///
/// \tparam T type to check for equality with the PLY data type \p type
/// \param type PLY data type to check for equality with `T`
/// \return \c true in case an type `T` equals the given PLY data type \p type
template<typename T>
constexpr bool isSame(PlyDataType type)
{
  switch (type)
  {
    case PlyDataType::Char:
      return std::is_same<T, char>::value;
    case PlyDataType::UChar:
      return std::is_same<T, unsigned char>::value;
    case PlyDataType::Short:
      return std::is_same<T, short>::value;
    case PlyDataType::UShort:
      return std::is_same<T, unsigned short>::value;
    case PlyDataType::Int:
      return std::is_same<T, int>::value;
    case PlyDataType::UInt:
      return std::is_same<T, unsigned int>::value;
    case PlyDataType::Float:
      return std::is_same<T, float>::value;
    case PlyDataType::Double:
      return std::is_same<T, double>::value;
  }

  return false;
}

/// Type function that returns whether a given type `T` is an instance of
/// `Pack<>`.
/// @{
template<typename T>
struct IsPack
{
  static constexpr bool value = false;
};

template<typename T, std::size_t N>
struct IsPack<reflect::Pack<T, N>>
{
  static constexpr bool value = true;
};
/// @}

/// Type function that returns the size of some type T, effectively implementing
/// sizeof(T), where it overrides sizeof() for types of instance Pack<>. For
/// pack types, the computed size is the sum of `SizeOf` for all types in the
/// pack, whereas for arrays it is the `SizeOf` of the element type in the array
/// times the number of elements in the array.
/// @{
template<typename T, typename... Ts>
struct SizeOf
{
  static constexpr auto size = SizeOf<T>::size + SizeOf<Ts...>::size;
};

template<typename T>
struct SizeOf<T>
{
  static constexpr auto size = sizeof(T);
};

template<typename T, std::size_t N>
struct SizeOf<reflect::Array<T, N>>
{
  static constexpr auto size = N * SizeOf<T>::size;
};

template<typename T, std::size_t N>
struct SizeOf<reflect::Pack<T, N>>
{
  static constexpr auto size = N * SizeOf<T>::size;
};
/// @}

/// Returns the size of some type T, effectively implementing sizeof(T), where
/// it overrides sizeof() for types of instance Pack<> and Array<>. For pack
/// types, the computed size is the sum of `SizeOf` for all types in the pack,
/// whereas for arrays it is the `SizeOf` of the element type in the array times
/// the number of elements in the array.
///
/// \return the sum of the size in bytes of the given list of reflection types
///     `Ts...`
template<typename... Ts>
constexpr std::size_t sizeOf()
{
  return SizeOf<Ts...>::size;
}

/// Returns the size in bytes of the given PLY data type.
///
/// \param type PLY data type to calculate the size for
/// \return the size in bytes of the given PLY data type
constexpr std::size_t sizeOf(PlyDataType type)
{
  switch (type)
  {
    case PlyDataType::Char:
    case PlyDataType::UChar:
      return 1;
    case PlyDataType::Short:
    case PlyDataType::UShort:
      return 2;
    case PlyDataType::Int:
    case PlyDataType::UInt:
    case PlyDataType::Float:
      return 4;
    case PlyDataType::Double:
      return 8;
  }

  return 0;
}

/// Aligns the given input pointer according to the size of the given PLY data
/// type.
///
/// \param ptr input point to align
/// \param type PLY data type of the object pointed to by \p ptr
/// \return the input pointer \p ptr aligned according to the alignment
///     requirements for the given PLY data type \p type
template<typename Ptr>
constexpr Ptr align(Ptr ptr, PlyDataType type)
{
  switch (type)
  {
    case PlyDataType::Char:
      return detail::align(ptr, alignof(char));
    case PlyDataType::UChar:
      return detail::align(ptr, alignof(unsigned char));
    case PlyDataType::Short:
      return detail::align(ptr, alignof(short));
    case PlyDataType::UShort:
      return detail::align(ptr, alignof(unsigned short));
    case PlyDataType::Int:
      return detail::align(ptr, alignof(int));
    case PlyDataType::UInt:
      return detail::align(ptr, alignof(unsigned int));
    case PlyDataType::Float:
      return detail::align(ptr, alignof(float));
    case PlyDataType::Double:
      return detail::align(ptr, alignof(double));
  }

  return ptr;
}

/// Type function that returns whether a type is aligned in memory at the given
/// memory \p offset.
///
/// \tparam T type to check whether it is aligned in memory at the given \p
///     offset
/// \param offset relative memory offset at which an object of type `T` is to be
///     stored
/// \return \c true in case the given type is packed, \c false otherwise
template<typename T>
constexpr bool isPacked(uintptr_t offset = 0)
{
  return (offset % alignof(T)) == 0;
}

/// Type function that returns whether a list of types are consecutively aligned
/// in memory, without any padding, at the given memory \p offset.
///
/// \param offset relative memory offset at which an object of type `T` is to be
///     stored
/// \return \c true in case the given type is packed, \c false otherwise
template<typename T, typename U, typename... Ts>
constexpr bool isPacked(uintptr_t offset = 0)
{
  return (offset % alignof(T)) == 0 && isPacked<U, Ts...>(offset + sizeOf<T>());
}

/// Type function that returns whether all types in the given list of types are
/// trivially copyable, that is, they do not have a custom copy constructor
/// non-standard implementation.
///
/// \tparam T,U,Ts... type for which to check whether it is trivially copyable
/// \return \c true in case all types passed in as template parameters are
///     trivially copyable, \c false otherwise
template<typename... Ts>
constexpr bool isTriviallyCopyable()
{
  return (std::is_trivially_copyable_v<Ts> && ...);
}

/// @{
/// Type that provides a function operator that returns whether a range of
/// properties in [`first`, `last`) represents PLY properties that can be
/// trivially copied to the given destination type `T`.
template<typename T>
struct IsMemcpyable
{
  bool operator()(const PlyPropertyConstIterator first, const PlyPropertyConstIterator last) const
  {
    return first < last && isSame<T>(first->type());
  }
};

template<typename T, std::size_t N>
struct IsMemcpyable<reflect::Array<T, N>>
{
  bool operator()(const PlyPropertyConstIterator, const PlyPropertyConstIterator) const { return false; }
};
/// @}

template<typename T, std::size_t N>
struct IsMemcpyable<reflect::Pack<T, N>>
{
  bool operator()(const PlyPropertyConstIterator first, const PlyPropertyConstIterator last) const
  {
    return first + N <= last &&
           std::all_of(first, first + N, [](const PlyProperty &p) { return isSame<T>(p.type()); });
  }
};

/// Returns whether the range of properties in [`first`, `last`) represents PLY
/// properties that have the same type as the given type range. Note that a
/// single `reflect::Pack` type is compared with same number of properties as
/// are in the pack.
///
/// \param first iterator pointing to the first property in the range of input
///     PLY properties
/// \param last iterator pointing after the last property in the range of input
///     PLY properties
/// \return \c true in case all PLY properties in the given range are
///     `memcpy`'able, \c false otherwise
template<typename T>
bool isMemcpyable(const PlyPropertyConstIterator first, const PlyPropertyConstIterator last)
{
  return first + detail::numProperties<T>() == last && IsMemcpyable<T>{}(first, last);
}

/// Returns whether the range of properties in [`first`, `last`) represents PLY
/// properties that have the same type as the given type range. Note that a
/// single `reflect::Pack` type is compared with same number of properties as
/// are in the pack.
///
/// \param first iterator pointing to the first property in the range of input
///     PLY properties
/// \param last iterator pointing after the last property in the range of input
///     PLY properties
/// \return \c true in case all PLY properties in the given range are
///     `memcpy`'able, \c false otherwise
template<typename T, typename U, typename... Ts>
bool isMemcpyable(const PlyPropertyConstIterator first, const PlyPropertyConstIterator last)
{
  return IsMemcpyable<T>{}(first, last) && isMemcpyable<U, Ts...>(first + detail::numProperties<T>(), last);
}

}

#endif
