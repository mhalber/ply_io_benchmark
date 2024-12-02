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

#ifndef PLYWOOT_HPP
#define PLYWOOT_HPP

/// \file

#include "plywoot/ascii_parser_policy.hpp"
#include "plywoot/ascii_writer_policy.hpp"
#include "plywoot/binary_parser_policy.hpp"
#include "plywoot/binary_writer_policy.hpp"
#include "plywoot/element_data.hpp"
#include "plywoot/header_parser.hpp"
#include "plywoot/parser.hpp"
#include "plywoot/parser_variant.hpp"
#include "plywoot/reflect.hpp"
#include "plywoot/type_traits.hpp"
#include "plywoot/types.hpp"
#include "plywoot/writer.hpp"
#include "plywoot/writer_variant.hpp"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <istream>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace plywoot {

/// Represents an input PLY data stream that can be queried for data.
class IStream
{
public:
  /// Constructs an input PLY data stream from the given input stream. This will
  /// automatically trigger parsing of all PLY data in the input stream.
  ///
  /// \param is input stream containing the PLY data to parse
  IStream(std::istream &is) : IStream{is, detail::HeaderParser{is}} {}

  /// Returns all comments embedded in the PLY header.
  ///
  /// \return all comments embedded in the PLY header
  const std::vector<Comment> &comments() const { return comments_; }

  /// Returns all elements associated with this PLY file.
  ///
  /// \return all elements associated with this PLY file
  const std::vector<PlyElement> &elements() const { return elements_; }

  /// Returns a pair where the first element is a copy of the element with the
  /// given name in case it exists. The second element is a boolean that
  /// indicates whether the requested element was found. In case a requested
  /// element was not found in the input data, a default constructed element
  /// is returned.
  ///
  /// \param name name of the element to retrieve
  /// \return a pair where the first element is a copy of the element in case it
  ///   exists, and the second element is a Boolean indicating whether the
  ///   requested element was found
  // TODO(ton): change return type to an optional.
  std::pair<PlyElement, bool> element(const std::string &name) const
  {
    const auto it = std::find_if(
        elements_.begin(), elements_.end(), [&](const PlyElement &e) { return e.name() == name; });
    return it != elements_.end() ? std::pair<PlyElement, bool>{*it, true}
                                 : std::pair<PlyElement, bool>{{}, false};
  }

  /// Returns the format of the input PLY data stream.
  ///
  /// \return the format of the input PLY data stream
  PlyFormat format() const { return format_; }

  /// Positions the read head at the start of the element with the given name,
  /// or at the end of the stream in case the given element is not present in
  /// the stream, skipping over elements that do not match \p elementName.
  ///
  /// \param elementName name of the element to find
  /// \return \c true in case an element with the given name was found,
  ///     \c false otherwise
  bool find(const std::string &elementName) const
  {
    while (currElement_ != elements_.end() && currElement_->name() != elementName) { skipElement(); }
    return currElement_ != elements_.end();
  }

  /// Returns a copy of the current element that can be either read or skipped.
  ///
  /// \return  a copy of the current element that can be either read or skipped
  PlyElement element() const { return hasElement() ? *currElement_ : PlyElement{}; }
  /// Returns whether there are still elements left to parse.
  ///
  /// \return \c true in case there are still elements left to parse, \c false
  ///     otherwise
  bool hasElement() const { return currElement_ != elements_.end(); }

  /// Reads an element to a newly allocated block of memory wrapped by a
  /// `PlyElementData` instance. PLY data types are directly mapped to their
  /// corresponding native types. Lists are mapped to an `std::vector<T>` where
  /// `T` is the type of type of the element in the lists. All data is aligned
  /// according to the alignment requirements of the compiler platform.
  ///
  /// \pre `hasElement()` must be \c true
  /// \return memory block in the form of a `PlyElementData` instance
  ///     representing all data for the active element to be parsed
  PlyElementData readElement() const { return parser_.read(*currElement_++); }

  /// Reads the current element from the PLY input data stream, returning a list
  /// of objects of type `T`, where the `Layout` type is used to identify how
  /// properties from the PLY element are mapped on objects of type `T`.
  ///
  /// \tparam T type of objects to be read from the stream
  /// \tparam Layout layout specifying the mapping of PLY properties to `T`
  /// \pre `hasElement()` must be \c true
  /// \return a vector of object of type `T` representing the element that was
  ///     parsed using the PLY property mapping embedded in the given `Layout`
  ///     type
  // TODO(ton): add more extensive documentation
  // TODO(ton): probably better to add another parameter 'size' to guard
  template<typename T, typename Layout>
  std::vector<T> readElement() const
  {
    std::vector<T> result(currElement_->size());
    parser_.read(*currElement_++, Layout{result});
    return result;
  }

  /// Skips the current element.
  void skipElement() const { parser_.skip(*currElement_++); }

private:
  /// Constructs a PLY file from the given input stream and header parser.
  ///
  /// \param is input stream containing the PLY data to parse
  /// \param headerParser header parser instance
  IStream(std::istream &is, const detail::HeaderParser &headerParser)
      : parser_{is, headerParser.format()},
        comments_{headerParser.comments()},
        elements_{headerParser.elements()},
        format_{headerParser.format()},
        currElement_{elements_.begin()}
  {
  }

  /// Variant type around some parser type that represents that correct type for
  /// the associated input PLY stream.
  detail::ParserVariant parser_;

  /// All comments defines in the header.
  std::vector<Comment> comments_;
  /// All PLY elements defined in the header.
  std::vector<PlyElement> elements_;
  /// Format of the PLY input data, either ASCII, little-, or big-endian binary.
  PlyFormat format_;

  /// Iterator pointing to the current element.
  mutable std::vector<PlyElement>::const_iterator currElement_;
};

/// Represents an output PLY data stream that can be used to output data to a
/// PLY format.
class OStream
{
public:
  /// Constructs an output PLY data stream of the given format type.
  ///
  /// \param format format of the output PLY data stream
  OStream(PlyFormat format) : format_{format} {}
  /// Constructs an output PLY data stream of the given format type, with the
  /// specified comments that should be written to the header. Line numbers of
  /// the comments should start from line 2, since according to the
  /// specification, comments may not occur in the first two lines of the
  /// header.
  ///
  /// \param format format of the output PLY data stream
  /// \param comments comments to store in the header of the output PLY data
  ///     stream
  OStream(PlyFormat format, std::vector<Comment> comments) : format_{format}, comments_{std::move(comments)}
  {
    // Ensure comments are sorted on line number (ascending).
    std::stable_sort(comments_.begin(), comments_.end(), [](const Comment &x, const Comment &y) {
      return x.line < y.line;
    });
  }

  /// Queues an element with the associated data for writing. Elements will be
  /// stored in the same order they are queued.
  ///
  /// \param element PLY element to add for streaming to this output stream
  /// \param layout PLY property to target type mapping representing the way PLY
  ///     properties should be mapped on the various target type member types
  template<typename... Ts>
  void add(const PlyElement &element, const reflect::Layout<Ts...> layout)
  {
    // Note; create a copy of the element that specifies as size the number of
    // items in the input layout.
    PlyElement layoutElement{element.name(), layout.size(), element.properties()};

    elementWriteClosures_.emplace_back(
        std::move(layoutElement),
        [this, layout](detail::WriterVariant &writer, const PlyElement &e) { writer.write(e, layout); });
  }

  /// Queues the given element data for writing. This takes ownership of the
  /// data to be written, to ensure it does not go out of scope prior to
  /// committing all data to the output stream through `write()`.
  ///
  /// \param elementData raw element data to stream to this output stream
  void add(const PlyElementData &elementData)
  {
    // TODO(ton): once we upgrade to C++17, move capture element data in the
    // lambda below.
    const std::uint8_t *src = elementData.data();
    const std::size_t alignment = elementData.alignment();

    elementWriteClosures_.emplace_back(
        elementData.element(), [this, src, alignment](detail::WriterVariant &writer, const PlyElement &e) {
          writer.write(e, src, alignment);
        });
  }

  /// Writes all data as a PLY file queued through `addElement()` to the given
  /// output stream.
  ///
  /// \param os output stream to write the queued element data to
  void write(std::ostream &os) const
  {
    writeHeader(os);

    detail::WriterVariant writer{os, format_};
    for (const auto &elementClosurePair : elementWriteClosures_)
    {
      const PlyElement &element{elementClosurePair.first};
      const auto &writeFn = elementClosurePair.second;
      writeFn(writer, element);
    }
  }

private:
  /// Writes the ASCII PLY header, which defines the format of the PLY data, the
  /// elements and element properties that occur in the data.
  ///
  /// \param os output stream to write the PLY header data to
  void writeHeader(std::ostream &os) const
  {
    os << "ply\n";

    switch (format_)
    {
      case PlyFormat::Ascii:
        os << "format ascii 1.0\n";
        break;
      case PlyFormat::BinaryBigEndian:
        os << "format binary_big_endian 1.0\n";
        break;
      case PlyFormat::BinaryLittleEndian:
        os << "format binary_little_endian 1.0\n";
        break;
    }

    // Maintain line number to be able to serialize comments at the right
    // location in the header. Comments may only occur after the 'ply' magic
    // 'number', and the format specification. Ideally this state should be
    // limited to the scope of the closure, but that is not supported in C++11.
    std::uint32_t line = 2;
    auto first = comments_.begin();
    const auto last = comments_.end();
    const auto maybeWriteComments = [&]() {
      while (first != last && first->line == line++)
      {
        if (first->text.empty()) { os << "comment\n"; }
        else { os << "comment " << first->text << '\n'; }
        ++first;
      }
    };

    for (const auto &elementClosurePair : elementWriteClosures_)
    {
      maybeWriteComments();
      const PlyElement &element{elementClosurePair.first};
      os << "element " << element.name() << ' ' << element.size() << '\n';

      for (const PlyProperty &property : element.properties())
      {
        maybeWriteComments();
        if (property.isList())
        {
          os << "property list " << property.sizeType() << ' ' << property.type() << ' ' << property.name()
             << '\n';
        }
        else { os << "property " << property.type() << ' ' << property.name() << '\n'; }
      }
    }

    maybeWriteComments();
    os << "end_header\n";
  }

  using ElementWriteClosure = std::function<void(detail::WriterVariant &, const PlyElement &)>;

  /// All queued elements with the associated data.
  std::vector<std::pair<PlyElement, ElementWriteClosure>> elementWriteClosures_;
  /// Format the PLY data should be written in.
  PlyFormat format_;
  /// Comments to write out to the PLY file. Invariant is that comments are
  /// sorted ascending based on their associated line number.
  std::vector<Comment> comments_;
};

/// Converts the given input PLY stream to the requested format, and outputs the
/// resulting PLY data to the given output stream.
///
/// \param is input stream containing the PLY data to convert
/// \param os output stream to write converted PLY data to
/// \param format PLY format of the output PLY data
inline void convert(std::istream &is, std::ostream &os, PlyFormat format)
{
  IStream plyIs{is};
  OStream plyOs{format};

  // Note; preserve element data until the write() function below finishes.
  std::vector<PlyElementData> elementsData;

  while (plyIs.hasElement())
  {
    const plywoot::PlyElement element = plyIs.element();
    elementsData.emplace_back(plyIs.readElement());
    plyOs.add(elementsData.back());
  }

  plyOs.write(os);
}

}

#endif
