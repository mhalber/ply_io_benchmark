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

#ifndef PLYWOOT_BUFFERED_ISTREAM_HPP
#define PLYWOOT_BUFFERED_ISTREAM_HPP

/// \file

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <istream>
#include <memory>

namespace {

/// Default buffer size; may need tweaking.
constexpr std::size_t IStreamBufferSize{1024 * 1024};

}

namespace plywoot::detail {

/// Wrapper around some input stream that provides buffered input functionality.
/// This will always buffer some compile-time given size of bytes up front, and
/// data is read from this buffer until the buffer is exhausted, at which point
/// it is refilled again with the next block of data from the wrapped input
/// stream. This improves reading from file-backed input streams considerably.
class BufferedIStream
{
public:
  /// Constructs a buffered input stream wrapper around the given input stream.
  explicit BufferedIStream(std::istream &is) : is_{is} { buffer(); }

  /// No copy semantics allowed.
  BufferedIStream(const BufferedIStream &) = delete;
  BufferedIStream &operator=(const BufferedIStream &) = delete;

  /// Returns whether the read head is at the end of the stream.
  bool eof() const { return *c_ == EOF; }

  /// Returns a raw character pointer representing the current read head in the
  /// stream.
  /// @{
  const char *data() const { return c_; }
  const char *&data() { return c_; }
  /// @}

  /// Reads an object of the given type from the input data stream.
  template<typename T>
  inline T read()
  {
    // Note; buffer a bit more than strictly necessary, so that we can move the
    // read head unconditionally after reading the object of type T.
    if (c_ + sizeof(T) > eob_) buffer(sizeof(T));
    const char *t = c_;
    c_ += sizeof(T);
    return *reinterpret_cast<const T *>(t);
  }

  /// Copies `n` bytes to the given destination buffer, assuming it may hold
  /// that many bytes. Returns a pointer pointing to one byte after the last
  /// byte that was copied to `dest`.
  inline std::uint8_t *memcpy(std::uint8_t *dest, std::size_t n)
  {
    if (n > IStreamBufferSize)
    {
      const std::size_t remaining = eob_ - c_;
      std::memcpy(dest, c_, remaining);
      is_.read(reinterpret_cast<char *>(dest) + remaining, n - remaining);
      c_ = eob_;
    }
    else
    {
      if (c_ + n > eob_) buffer(n);

      std::memcpy(dest, c_, n);
      c_ += n;
    }

    return dest + n;
  }

  /// Reads `N` objects of the given type `From` from the input data stream, and
  /// stores them contiguously at the given destination in memory as numbers of
  /// type `To`.
  template<typename From, typename To, std::size_t N>
  std::uint8_t *read(std::uint8_t *dest)
  {
    if constexpr (std::is_same_v<From, To>)
    {
      return this->memcpy(dest, N * sizeof(From));
    }
    else
    {
      static_assert(
          N * sizeof(From) <= IStreamBufferSize,
          "input stream buffer size is too small; increase IStreamBufferSize");

      constexpr std::size_t bytesToRead = N * sizeof(From);
      if (c_ + bytesToRead > eob_) buffer(bytesToRead);

      const From *from = reinterpret_cast<const From *>(c_);
      To *to = reinterpret_cast<To *>(dest);
      for (std::size_t i = 0; i < N; ++i) { *to++ = *from++; }

      c_ += bytesToRead;

      return reinterpret_cast<std::uint8_t *>(to);
    }
  }

  /// Skips the given number of bytes in the input stream.
  void skip(std::size_t n)
  {
    const std::size_t remaining = eob_ - c_;
    if (remaining > n) { c_ += n; }
    else
    {
      is_.seekg(n - remaining, std::ios_base::cur);
      buffer();
    }
  }

  /// Skips `n` lines in the input, places the read head at the first
  /// character after the `n`-th newline character that as found in the input,
  /// or in case that character does not exist, at EOF.
  void skipLines(std::size_t n)
  {
    while (*c_ != EOF && n > 0)
    {
      c_ = static_cast<const char *>(std::memchr(c_, '\n', eob_ - c_));
      if (c_)
      {
        readCharacter();
        --n;
      }
      else { buffer(); }
    }
  }

  /// Skips whitespace in the input stream, and positions the read head on the
  /// first non-whitespace character in the input stream relative from the
  /// current read head.
  inline void skipWhitespace()
  {
    while (0 <= *c_ && *c_ <= 0x20) { readCharacter(); }
  }

  /// Skips non-whitespace in the input stream, and positions the read head on
  /// the first whitespace character in the input stream relative from the
  /// current read head.
  inline void skipNonWhitespace()
  {
    while (*c_ > 0x20) readCharacter();
  }

  /// Ensures that the buffer contains at least the given number of characters.
  /// In case it already does, this does nothing, otherwise, it will shift the
  /// data remaining in the buffer to the front, then refill the remaining part
  /// of the buffer.
  void buffer(std::size_t minimum)
  {
    std::size_t remaining = eob_ - c_;
    if (remaining < minimum)
    {
      std::memcpy(buffer_.get(), c_, remaining);
      if (!is_.read(buffer_.get() + remaining, IStreamBufferSize - remaining))
      {
        // In case the buffer is only partially filled, fill the remainder with
        // EOF characters.
        remaining += is_.gcount();
        std::fill_n(buffer_.get() + remaining, IStreamBufferSize - remaining, EOF);
      }

      c_ = buffer_.get();
    }
  }

private:
  /// Unconditionally buffers data from the input stream.
  void buffer()
  {
    if (!is_.read(buffer_.get(), IStreamBufferSize))
    {
      // In case the buffer is only partially filled, fill the remainder with
      // EOF characters.
      auto remaining = is_.gcount();
      std::fill_n(buffer_.get() + remaining, IStreamBufferSize - remaining, EOF);
    }

    c_ = buffer_.get();
  }

  /// Reads the next character in the input stream and advances the read head by
  /// one character.
  inline void readCharacter()
  {
    if (c_++ == eob_) { buffer(); }
  }

  /// Buffered data, always a null terminated string.
  std::unique_ptr<char[]> buffer_{new char[IStreamBufferSize]};

  /// Character the read head is currently pointing to. Invariant:
  ///
  ///       buffer_ <= c_ <= (buffer_ + sizeof(buffer_))
  ///
  /// Note that the invariant allows for one character lookahead without the
  /// need to check whether we need to read data from disk.
  const char *c_{buffer_.get() + IStreamBufferSize};
  /// Number of bytes remaining in the buffer.
  const char *eob_{buffer_.get() + IStreamBufferSize};

  /// Reference to the wrapped standard input stream.
  std::istream &is_;
};

}

#endif
