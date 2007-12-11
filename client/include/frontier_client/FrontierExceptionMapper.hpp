/*
 * frontier client C++ API exception mapper header
 *
 * Author: Sinisa Vaseli
 *
 * $Id$
 *
 *  Copyright (C) 2007  Fermilab
 *
 *  This program is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License
 *  as published by the Free Software Foundation, either version 3 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this program.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */
#
#ifndef FRONTIER_EXCEPTION_MAP_HPP
#define FRONTIER_EXCEPTION_MAP_HPP

#include <string>
#include <iostream>
#include <exception>

#include "frontier_client/FrontierException.hpp"

namespace frontier {

  class FrontierExceptionMapper {
  private:
    FrontierExceptionMapper() {}
  public:
    static void throwException(int errorCode, const std::string& errorMessage);
  };

} // namespace frontier

#endif // ifndef FRONTIER_EXCEPTION_MAP_HPP


