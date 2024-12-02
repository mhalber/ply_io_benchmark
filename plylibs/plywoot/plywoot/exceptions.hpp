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

#ifndef PLYWOOT_EXCEPTIONS_HPP
#define PLYWOOT_EXCEPTIONS_HPP

/// \file

#include <stdexcept>
#include <string>

namespace plywoot {

/// Base class for all exceptions thrown by PLYwoot.
class Exception : public std::runtime_error
{
public:
  /// Constructs an exception instance with the given error message.
  ///
  /// \param message exception message to store in this exception
  Exception(const std::string &message) : std::runtime_error(message) {}
};

}

#endif
