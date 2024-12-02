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

#ifndef PLYWOOT_ELEMENT_DATA_HPP
#define PLYWOOT_ELEMENT_DATA_HPP

/// \file

#include "types.hpp"

#include <algorithm>
#include <memory>
#include <vector>

namespace plywoot {

/// Abstracts a block of memory representing storing all data in a PLY element
/// mapping the data types used in the PLY file directly on native types.
class PlyElementData
{
public:
  /// Default constructor.
  PlyElementData() = default;

  /// Constructs an element data block for the given PLY element. Calculates the
  /// total size in bytes of the data block to represent all data in the given
  /// PLY element. Note that lists are mapped on to `std::vector`, so we can
  /// pre-calculate the data size of the data block.
  ///
  /// \param element PLY element to construct this instance for
  explicit PlyElementData(const PlyElement &element) : element_{element}
  {
    // Keep track of the alignment requirements of the memory block that will
    // store all element data. Store the relative offsets of every manually
    // allocated `std::vector` in the element data.
    const std::vector<PlyProperty> &properties = element_.properties();
    for (const PlyProperty &property : properties)
    {
      if (property.isList())
      {
        // Note, the exact type of the vector element does not matter here.
        listOffsets_.push_back(detail::align(bytesPerElement_, alignof(std::vector<int>)));
        bytesPerElement_ = listOffsets_.back() + sizeof(std::vector<int>);
        alignment_ = std::max(alignment_, alignof(std::vector<int>));
      }
      else
      {
        bytesPerElement_ = detail::align(bytesPerElement_, property.type()) + detail::sizeOf(property.type());
        switch (property.type())
        {
          case PlyDataType::Short:
            alignment_ = std::max(alignment_, alignof(short));
            break;
          case PlyDataType::UShort:
            alignment_ = std::max(alignment_, alignof(unsigned short));
            break;
          case PlyDataType::Int:
            alignment_ = std::max(alignment_, alignof(int));
            break;
          case PlyDataType::UInt:
            alignment_ = std::max(alignment_, alignof(unsigned int));
            break;
          case PlyDataType::Float:
            alignment_ = std::max(alignment_, alignof(float));
            break;
          case PlyDataType::Double:
            alignment_ = std::max(alignment_, alignof(double));
            break;
          default:
            break;
        }
      }
    }

    // Consecutive instances of an element in the memory block are aligned given
    // the maximum alignment requirement of an individual property type in an
    // element. The overall alignment requirements of an element determine the
    // overall size of the element in bytes.
    bytesPerElement_ = detail::align(bytesPerElement_, alignment_);

    // Now, allocate a raw block of memory large enough to hold all element
    // data.
    data_ = std::unique_ptr<std::uint8_t[]>(new std::uint8_t[element_.size() * bytesPerElement_]);

    // Finally, allocate all vectors holding the variable length list
    // properties.
    auto listOffset = listOffsets_.begin();
    for (PlyProperty property : element_.properties())
    {
      if (property.isList())
      {
        // Note, the exact type of the vector element does not matter here.
        std::uint8_t *ptr = data_.get() + *listOffset++;
        for (std::size_t i = 0; i < element_.size(); ++i, ptr += bytesPerElement_)
        {
          switch (property.type())
          {
            case PlyDataType::Char:
              new (ptr) std::vector<char>();
              break;
            case PlyDataType::UChar:
              new (ptr) std::vector<unsigned char>();
              break;
            case PlyDataType::Short:
              new (ptr) std::vector<short>();
              break;
            case PlyDataType::UShort:
              new (ptr) std::vector<unsigned short>();
              break;
            case PlyDataType::Int:
              new (ptr) std::vector<int>();
              break;
            case PlyDataType::UInt:
              new (ptr) std::vector<unsigned int>();
              break;
            case PlyDataType::Float:
              new (ptr) std::vector<float>();
              break;
            case PlyDataType::Double:
              new (ptr) std::vector<double>();
              break;
          }
        }
      }
    }
  }

  /// Destructor, deallocates all vectors holding the variable length lists.
  ~PlyElementData()
  {
    auto listOffset = listOffsets_.begin();
    for (const PlyProperty &property : element_.properties())
    {
      if (property.isList())
      {
        // Note, the exact type of the vector element does not matter here.
        std::uint8_t *ptr = data_.get() + *listOffset++;
        for (std::size_t i = 0; i < element_.size(); ++i, ptr += bytesPerElement_)
        {
          switch (property.type())
          {
            case PlyDataType::Char:
              reinterpret_cast<std::vector<char> *>(ptr)->~vector<char>();
              break;
            case PlyDataType::UChar:
              reinterpret_cast<std::vector<unsigned char> *>(ptr)->~vector<unsigned char>();
              break;
            case PlyDataType::Short:
              reinterpret_cast<std::vector<short> *>(ptr)->~vector<short>();
              break;
            case PlyDataType::UShort:
              reinterpret_cast<std::vector<unsigned short> *>(ptr)->~vector<unsigned short>();
              break;
            case PlyDataType::Int:
              reinterpret_cast<std::vector<int> *>(ptr)->~vector<int>();
              break;
            case PlyDataType::UInt:
              reinterpret_cast<std::vector<unsigned int> *>(ptr)->~vector<unsigned int>();
              break;
            case PlyDataType::Float:
              reinterpret_cast<std::vector<float> *>(ptr)->~vector<float>();
              break;
            case PlyDataType::Double:
              reinterpret_cast<std::vector<double> *>(ptr)->~vector<double>();
              break;
          }
        }
      }
    }
  }

  // This type is non-copyable.
  /// \cond
  PlyElementData(const PlyElementData &) = delete;
  PlyElementData &operator=(const PlyElementData &) = delete;
  /// \endcond

  /// Move constructor implemented in terms of move assignment for simplicity.
  ///
  /// \param x PLY element data instance to construct from
  PlyElementData(PlyElementData &&x) { *this = std::move(x); }

  /// Move assignment operator.
  ///
  /// \param x PLY element data instance to move assign from
  /// \return a reference to this newly assigned element data instance
  PlyElementData &operator=(PlyElementData &&x)
  {
    element_ = std::move(x.element_);
    data_.reset(x.data_.release());
    listOffsets_ = std::move(x.listOffsets_);
    bytesPerElement_ = x.bytesPerElement_;
    alignment_ = x.alignment_;

    x.bytesPerElement_ = 0;
    x.listOffsets_.clear();
    x.element_ = PlyElement();

    return *this;
  }

  /// Returns the associated element definition.
  ///
  /// \return the definition of the element associated with this element data
  ///     instance
  const PlyElement &element() const { return element_; }

  /// Returns a pointer to the memory block storing element data.
  ///
  /// \return a pointer to the memory block storing element data
  std::uint8_t *data() const { return data_.get(); }

  /// Alignment requirements of the elements stored in this memory block in
  /// bytes.
  ///
  /// \return alignment requirements of the elements stored in this memory
  ///     block in bytes
  std::size_t alignment() const { return alignment_; }

private:
  /// Definition of the element associated with this element data instance.
  PlyElement element_;
  /// Memory block containing all properties of the elements to store
  std::unique_ptr<std::uint8_t[]> data_;
  /// Relative offset of every `std::vector` instance created for variable
  /// lists.
  std::vector<std::size_t> listOffsets_;
  /// Number of bytes required to store a single element.
  std::size_t bytesPerElement_ = 0;
  /// Alignment requirements of the elements stored in this memory block in
  /// bytes.
  std::size_t alignment_ = alignof(char);
};

}

#endif
