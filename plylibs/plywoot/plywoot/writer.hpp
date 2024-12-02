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

#ifndef PLYWOOT_WRITER_HPP
#define PLYWOOT_WRITER_HPP

/// \file

#include "reflect.hpp"
#include "std.hpp"
#include "types.hpp"

#include <cstdint>
#include <type_traits>

namespace plywoot::detail {

/// Represents a generic PLY writer that is parameterized with format specific
/// functionality through the FormatWriterPolicy type, which should adhere to
/// the following model requirements:
///
///   - template<typename T> void writeNumber(T t);
///   - template<typename PlySizeT, typename PlyT, typename SrcT>
///     void writeList(const SrcT *t, std::size_t n) const;
///   - template<typename PlyT, typename SrcT>
///     void writeNumbers(const SrcT *t, std::size_t n) const;
///
///   - void writeNewline();
///   - void writeTokenSeparator();
template<typename FormatWriterPolicy>
class Writer : private FormatWriterPolicy
{
public:
  using FormatWriterPolicy::FormatWriterPolicy;

  /// Writes a PLY element to the associated output stream, assuming property
  /// types should be mapped directly to their corresponding native types. This
  /// is used for writing `PlyElementData` instances.
  void write(const PlyElement &element, const std::uint8_t *src, std::size_t alignment) const
  {
    for (std::size_t i = 0; i < element.size(); ++i)
    {
      const std::vector<PlyProperty> &properties = element.properties();
      const auto first = properties.begin();
      const auto last = properties.end();

      auto curr = first;
      while (curr < last)
      {
        if (curr > first) { this->writeTokenSeparator(); }

        const PlyProperty &property = *curr++;
        if (property.isList())
        {
          switch (property.type())
          {
            case PlyDataType::Char:
              src = writeProperty(src, property, reflect::Type<std::vector<char>>{});
              break;
            case PlyDataType::UChar:
              src = writeProperty(src, property, reflect::Type<std::vector<unsigned char>>{});
              break;
            case PlyDataType::Short:
              src = writeProperty(src, property, reflect::Type<std::vector<short>>{});
              break;
            case PlyDataType::UShort:
              src = writeProperty(src, property, reflect::Type<std::vector<unsigned short>>{});
              break;
            case PlyDataType::Int:
              src = writeProperty(src, property, reflect::Type<std::vector<int>>{});
              break;
            case PlyDataType::UInt:
              src = writeProperty(src, property, reflect::Type<std::vector<unsigned int>>{});
              break;
            case PlyDataType::Float:
              src = writeProperty(src, property, reflect::Type<std::vector<float>>{});
              break;
            case PlyDataType::Double:
              src = writeProperty(src, property, reflect::Type<std::vector<double>>{});
              break;
          }
        }
        else
        {
          switch (property.type())
          {
            case PlyDataType::Char:
              src = writeProperty(src, property, reflect::Type<char>{});
              break;
            case PlyDataType::UChar:
              src = writeProperty(src, property, reflect::Type<unsigned char>{});
              break;
            case PlyDataType::Short:
              src = writeProperty(src, property, reflect::Type<short>{});
              break;
            case PlyDataType::UShort:
              src = writeProperty(src, property, reflect::Type<unsigned short>{});
              break;
            case PlyDataType::Int:
              src = writeProperty(src, property, reflect::Type<int>{});
              break;
            case PlyDataType::UInt:
              src = writeProperty(src, property, reflect::Type<unsigned int>{});
              break;
            case PlyDataType::Float:
              src = writeProperty(src, property, reflect::Type<float>{});
              break;
            case PlyDataType::Double:
              src = writeProperty(src, property, reflect::Type<double>{});
              break;
          }
        }
      }

      src = detail::align(src, alignment);

      this->writeNewline();
    }
  }

  template<typename... Ts>
  void write(const PlyElement &element, const reflect::Layout<Ts...> layout) const
  {
    const auto first = element.properties().begin();
    const auto last = element.properties().end();

    const std::uint8_t *src = layout.data();
    for (std::size_t i = 0; i < layout.size(); ++i)
    {
      src = detail::align(writeElement<Ts...>(src, first, last), layout.alignment());
    }
  }

private:
  template<typename PlyT, typename SrcT>
  const std::uint8_t *writeProperty(const std::uint8_t *src, reflect::Type<SrcT>) const
  {
    if constexpr (std::is_arithmetic_v<SrcT>)
    {
      src = static_cast<const std::uint8_t *>(detail::align(src, alignof(SrcT)));
      this->writeNumber(static_cast<PlyT>(*reinterpret_cast<const SrcT *>(src)));
      return src + sizeof(SrcT);
    }
    else { return static_cast<const std::uint8_t *>(detail::align(src, alignof(SrcT))) + sizeof(SrcT); }
  }

  /// Specialization for the meta property type `Pack<T, N>`, this will write N
  /// numbers of type `T` to the output stream.
  template<typename PlyT, typename SrcT, std::size_t N>
  const std::uint8_t *writeProperty(const std::uint8_t *src, reflect::Type<reflect::Pack<SrcT, N>>) const
  {
    static_assert(N > 0, "invalid pack size specified (needs to be larger than zero)");
    static_assert(
        std::is_arithmetic<SrcT>::value, "it is not (yet) possible to write packs of non-numeric types");

    src = static_cast<const std::uint8_t *>(detail::align(src, alignof(SrcT)));
    this->template writeNumbers<PlyT, SrcT>(reinterpret_cast<const SrcT *>(src), N);
    return src + N * sizeof(SrcT);
  }

  /// Specialization for the meta property type `Stride<T>`, this will just skip
  /// over a member variable of type `T` in the source buffer.
  template<typename SrcT>
  const std::uint8_t *writeProperty(const std::uint8_t *src, reflect::Type<reflect::Stride<SrcT>>) const
  {
    return static_cast<const std::uint8_t *>(detail::align(src, alignof(SrcT))) + sizeof(SrcT);
  }

  /// Specialization for the meta property type `Array<T, N, SizeT>`, this will
  /// write a list of N properties of type T.
  template<typename PlyT, typename PlySizeT, typename SrcT, std::size_t N>
  const std::uint8_t *writeListProperty(const std::uint8_t *src, reflect::Type<reflect::Array<SrcT, N>>) const
  {
    static_assert(N > 0, "invalid array size specified (needs to be larger than zero)");

    src = static_cast<const std::uint8_t *>(detail::align(src, alignof(SrcT)));
    this->template writeList<PlySizeT, PlyT, SrcT>(reinterpret_cast<const SrcT *>(src), N);
    return src + N * sizeof(SrcT);
  }

  /// Specialization for a vector of type `T`.
  template<typename PlyT, typename PlySizeT, typename SrcT>
  const std::uint8_t *writeListProperty(const std::uint8_t *src, reflect::Type<std::vector<SrcT>>) const
  {
    src = static_cast<const std::uint8_t *>(detail::align(src, alignof(std::vector<SrcT>)));
    const std::vector<SrcT> &v = *reinterpret_cast<const std::vector<SrcT> *>(src);
    this->template writeList<PlySizeT, PlyT, SrcT>(v.data(), v.size());
    return src + sizeof(std::vector<SrcT>);
  }

  template<typename PlyT, typename TypeTag>
  const std::uint8_t *writeListProperty(const std::uint8_t *src, const PlyProperty &property, TypeTag tag)
      const
  {
    switch (property.sizeType())
    {
      case PlyDataType::Char:
        return writeListProperty<PlyT, char>(src, tag);
      case PlyDataType::UChar:
        return writeListProperty<PlyT, unsigned char>(src, tag);
      case PlyDataType::Short:
        return writeListProperty<PlyT, short>(src, tag);
      case PlyDataType::UShort:
        return writeListProperty<PlyT, unsigned short>(src, tag);
      case PlyDataType::Int:
        return writeListProperty<PlyT, int>(src, tag);
      case PlyDataType::UInt:
        return writeListProperty<PlyT, unsigned int>(src, tag);
      case PlyDataType::Float:
        return writeListProperty<PlyT, float>(src, tag);
      case PlyDataType::Double:
        return writeListProperty<PlyT, double>(src, tag);
    }

    return src;
  }

  template<typename TypeTag>
  const std::uint8_t *writeProperty(const std::uint8_t *src, const PlyProperty &property, TypeTag tag) const
  {
    if constexpr (detail::isList<TypeTag>())
    {
      switch (property.type())
      {
        case PlyDataType::Char:
          return writeListProperty<char>(src, property, tag);
        case PlyDataType::UChar:
          return writeListProperty<unsigned char>(src, property, tag);
        case PlyDataType::Short:
          return writeListProperty<short>(src, property, tag);
        case PlyDataType::UShort:
          return writeListProperty<unsigned short>(src, property, tag);
        case PlyDataType::Int:
          return writeListProperty<int>(src, property, tag);
        case PlyDataType::UInt:
          return writeListProperty<unsigned int>(src, property, tag);
        case PlyDataType::Float:
          return writeListProperty<float>(src, property, tag);
        case PlyDataType::Double:
          return writeListProperty<double>(src, property, tag);
      }
    }
    else
    {
      switch (property.type())
      {
        case PlyDataType::Char:
          return writeProperty<char>(src, tag);
        case PlyDataType::UChar:
          return writeProperty<unsigned char>(src, tag);
        case PlyDataType::Short:
          return writeProperty<short>(src, tag);
        case PlyDataType::UShort:
          return writeProperty<unsigned short>(src, tag);
        case PlyDataType::Int:
          return writeProperty<int>(src, tag);
        case PlyDataType::UInt:
          return writeProperty<unsigned int>(src, tag);
        case PlyDataType::Float:
          return writeProperty<float>(src, tag);
        case PlyDataType::Double:
          return writeProperty<double>(src, tag);
      }
    }

    return src;
  }

  /// Writes a single object of type `SrcT` read from the input buffer `src` to
  /// the given output stream `os` in case a corresponding element property is
  /// defined for it (`first` < `last`). Otherwise, it will skip over the object
  /// in the input buffer.
  template<typename SrcT>
  const std::uint8_t *writeProperty(
      const std::uint8_t *src,
      PlyPropertyConstIterator first,
      PlyPropertyConstIterator last) const
  {
    return first < last ? writeProperty(src, *first, reflect::Type<SrcT>{})
                        : writeProperty(src, reflect::Type<reflect::Stride<SrcT>>{});
  }

  /// Reads a list of objects of type `T`, `U`, and `Ts...` from the input
  /// buffer `src` and writes them to the given output stream `os`. The objects
  /// should have a corresponding property defined in the input buffer, the list
  /// of properties is defined by the property range (`first`, `last`].
  /// @{
  template<typename Policy, typename T>
  const std::uint8_t *writeProperties(
      const std::uint8_t *src,
      PlyPropertyConstIterator first,
      PlyPropertyConstIterator last) const
  {
    return writeProperty<T>(src, first, last);
  }

  template<typename Policy, typename T, typename U, typename... Ts>
  const std::uint8_t *writeProperties(
      const std::uint8_t *src,
      PlyPropertyConstIterator first,
      PlyPropertyConstIterator last) const
  {
    if constexpr (std::is_same_v<Policy, AsciiWriterPolicy>)
    {
      src = writeProperty<T>(src, first, last);
      first += detail::numProperties<T>();
      if (first < last) { this->writeTokenSeparator(); }
      return writeProperties<Policy, U, Ts...>(src, first, last);
    }
    else
    {
      return writeProperties<Policy, U, Ts...>(
          writeProperty<T>(src, first, last), first + detail::numProperties<T>(), last);
    }
  }
  /// @}

  template<typename... Ts>
  const std::uint8_t *writeElement(
      const std::uint8_t *src,
      PlyPropertyConstIterator first,
      PlyPropertyConstIterator last) const
  {
    src = this->writeProperties<FormatWriterPolicy, Ts...>(src, first, last);

    // In case the element defines more properties than the source data,
    // append the missing properties with a default value of zero.
    if (detail::numProperties<Ts...>() < static_cast<std::size_t>(std::distance(first, last)))
    {
      this->writeMissingProperties(first + detail::numProperties<Ts...>(), last);
    }

    this->writeNewline();

    return src;
  }
};

}

#endif
