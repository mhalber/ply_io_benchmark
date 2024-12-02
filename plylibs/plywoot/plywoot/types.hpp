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

#ifndef PLYWOOT_TYPES_HPP
#define PLYWOOT_TYPES_HPP

/// \file

#include <algorithm>
#include <cstdint>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace plywoot {

/// Enumeration of data types supported by the PLY format.
enum class PlyDataType { Char, UChar, Short, UShort, Int, UInt, Float, Double };

/// Outputs a textual representation of the given PLY data type \p type.
///
/// \param os output stream to output the textual representation of \p type to
/// \param type PLY data type enumeration value to output a textual
///     representation for
/// \return the output stream \p os
inline std::ostream &operator<<(std::ostream &os, PlyDataType type)
{
  switch (type)
  {
    case PlyDataType::Char:
      return os << "char";
    case PlyDataType::UChar:
      return os << "uchar";
    case PlyDataType::Short:
      return os << "short";
    case PlyDataType::UShort:
      return os << "ushort";
    case PlyDataType::Int:
      return os << "int";
    case PlyDataType::UInt:
      return os << "uint";
    case PlyDataType::Float:
      return os << "float";
    case PlyDataType::Double:
      return os << "double";
  }

  return os;
}

/// Enumeration of all formats supported by the PLY format. This represents the format
/// of the data stored in the PLY file, the header is always human-readable
/// ASCII text.
enum class PlyFormat {
  Ascii,
  BinaryBigEndian,
  BinaryLittleEndian,
};

/// Represents a PLY property stored in an element. A PLY property is a named
/// data element with a certain type.
struct PlyProperty
{
  /// Default constructor.
  PlyProperty() = default;
  /// Constructs a PLY property for a property with the given name and type.
  ///
  /// \param name name of the PLY property
  /// \param type type of the PLY property
  PlyProperty(std::string name, PlyDataType type) : name_{std::move(name)}, type_{type} {}
  /// Constructs a PLY list property for a list property with the given name,
  /// type, and size type.
  ///
  /// \param name name of the PLY list property
  /// \param type type of the elements in the PLY property
  /// \param sizeType type of number storing the size of list
  PlyProperty(std::string name, PlyDataType type, PlyDataType sizeType)
      : name_{std::move(name)}, type_{type}, isList_{true}, sizeType_{sizeType}
  {
  }

  /// Returns the name of this property.
  ///
  /// \return the name of this property
  const std::string &name() const { return name_; }
  /// Returns the type of this property.
  ///
  /// \return the type of this property
  PlyDataType type() const { return type_; }
  /// Returns whether this property represents a list property.
  ///
  /// \return \c true in case this property represents a list property, \c false
  ///   otherwise
  bool isList() const { return isList_; }
  /// Returns the size type of this property.
  ///
  /// \return the size type of this property
  PlyDataType sizeType() const { return sizeType_; }

  /// Compares the two given PLY properties \p x and \p y for equality.
  ///
  /// \param x left-hand side property to compare for equality with \p y
  /// \param y right-hand side property to compare for equality with \p x
  /// \return \c true in case \p x is equal to \p y, \c false otherwise
  inline friend bool operator==(const PlyProperty &x, const PlyProperty &y)
  {
    return x.type_ == y.type_ && x.isList_ == y.isList_ && x.sizeType_ == y.sizeType_ && x.name_ == y.name_;
  }

  /// Compares the two given PLY properties \p x and \p y for inequality.
  ///
  /// \param x left-hand side property to compare for inequality with \p y
  /// \param y right-hand side property to compare for inequality with \p x
  /// \return \c true in case \p x is not equal to \p y, \c false otherwise
  inline friend bool operator!=(const PlyProperty &x, const PlyProperty &y) { return !(x == y); }

private:
  std::string name_;
  PlyDataType type_;

  bool isList_{false};
  PlyDataType sizeType_{PlyDataType::Char};
};

/// Convenience type alias for a const vector iterator pointing to a
/// `PlyProperty`.
using PlyPropertyConstIterator = std::vector<PlyProperty>::const_iterator;

/// Represents an element stored in a PLY file. An element is a named collection
/// of ordered PLY properties.
struct PlyElement
{
  /// Default constructor.
  PlyElement() = default;
  /// Constructor taking a name and a list of initial properties to associate
  /// with this element.
  ///
  /// \param name name of the PLY element to construct
  /// \param properties definitions of the PLY properties embedded in this
  ///     PLY element
  PlyElement(std::string name, std::vector<PlyProperty> properties)
      : name_{std::move(name)}, size_{0}, properties_{std::move(properties)}
  {
  }
  /// Constructor taking a name and size for this element.
  ///
  /// \param name name of the PLY element to construct
  /// \param size TODO
  PlyElement(std::string name, std::size_t size) : name_{std::move(name)}, size_{size} {}
  /// Constructor taking a name and size for this element, as well as a list
  /// of initial properties to associate with this element.
  ///
  /// \param name name of the PLY element to construct
  /// \param size TODO
  /// \param properties definitions of the PLY properties embedded in this
  ///     PLY element
  PlyElement(std::string name, std::size_t size, std::vector<PlyProperty> properties)
      : name_{std::move(name)}, size_{size}, properties_{std::move(properties)}
  {
  }

  /// Returns the name of this element.
  ///
  /// \return the name of this element
  const std::string &name() const { return name_; }
  /// Returns the size of this element.
  ///
  /// \return the size of this element
  // TODO(ton): remove from this type?
  std::size_t size() const { return size_; }
  /// Returns the properties associated with this element.
  ///
  /// \return the properties associated with this element
  const std::vector<PlyProperty> &properties() const { return properties_; }

  /// Returns a pair where the first element is a copy of the property with
  /// the given name in case it exists. The second element is a Boolean that
  /// indicates whether the requested property was found. In case a requested
  /// property was not found for this element, a default constructed property
  /// is returned.
  ///
  /// \param propertyName name of the property to find the definition for
  /// \return a pair where the first element is a copy of the property with
  ///     the given name in case it exists and the second element is a Boolean
  ///     that indicates whether the requested property was found
  // TODO(ton): return an optional
  std::pair<PlyProperty, bool> property(const std::string &propertyName) const
  {
    const auto it = std::find_if(properties_.begin(), properties_.end(), [&](const PlyProperty &p) {
      return p.name() == propertyName;
    });
    return it != properties_.end() ? std::pair<PlyProperty, bool>{*it, true}
                                   : std::pair<PlyProperty, bool>{{}, false};
  }

  /// Factory method that constructs a new PLY property definition associated
  /// with this PLY element in-place, forwarding the given arguments to the
  /// constructor of `plywoot::PlyProperty`.
  ///
  /// \param args arguments to forward to the constructor of `plywoot::Property`
  /// \return the newly constructed and added PLY property definition
  template<typename... Args>
  PlyProperty &addProperty(Args &&...args)
  {
    properties_.emplace_back(std::forward<Args>(args)...);
    return properties_.back();
  }

  /// Compares the two given PLY element definitions for equality.
  ///
  /// \param x left-hand side element definition to compare for equality with \a
  ///     y
  /// \param y right-hand side element definition to compare for equality with
  ///     \p x
  /// \return \c true in case \p x is equal to \p y, \c false otherwise
  inline friend bool operator==(const PlyElement &x, const PlyElement &y)
  {
    return x.size_ == y.size_ && x.name_ == y.name_ && x.properties_ == y.properties_;
  }
  /// Compares the two given PLY element definitions for inequality.
  ///
  /// \param x left-hand side element definition to compare for inequality with \a
  ///     y
  /// \param y right-hand side element definition to compare for inequality with
  ///     \p x
  /// \return \c true in case \p x is not equal to \p y, \c false otherwise
  inline friend bool operator!=(const PlyElement &x, const PlyElement &y) { return !(x == y); }

private:
  /// Name of this element.
  std::string name_;
  /// Size of this element.
  std::size_t size_;
  /// The definitions of the properties contained in this element.
  std::vector<PlyProperty> properties_;
};

/// A comment represents a single line of comment in some PLY file, with an
/// associated line number.
struct Comment
{
  /// Line number in the PLY header where this comment originates from.
  std::uint32_t line;
  /// The comment text.
  std::string text;

  /// Compares the two given comments \p x and \p p for equality.
  ///
  /// \param x left-hand side comment to check for equality
  /// \param y right-hand side comment to check for equality
  inline friend bool operator==(const Comment &x, const Comment &y)
  {
    return x.line == y.line && x.text == y.text;
  }
  /// Compares the two given comments \p x and \p y for inequality.
  ///
  /// \param x left-hand side comment to check for inequality
  /// \param y right-hand side comment to check for inequality
  inline friend bool operator!=(const Comment &x, const Comment &y) { return !(x == y); }
};

}

#endif
