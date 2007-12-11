/*
 * frontier client C++ exception mapper
 *
 * Author: Sinisa Veseli
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
#include <string>
#include <iostream>
#include <exception>

#include "frontier_client/FrontierException.hpp"
#include "frontier_client/FrontierExceptionMapper.hpp"

namespace frontier {

void FrontierExceptionMapper::throwException(
  int errorCode, const std::string& errorMessage) {
  switch(errorCode) {
    case FrontierErrorCode_InvalidArgument:
      throw InvalidArgument(errorMessage);
    case FrontierErrorCode_MemoryAllocationFailed:
      throw MemoryAllocationFailed(errorMessage);
    case FrontierErrorCode_ConfigurationError:
      throw ConfigurationError(errorMessage);
    case FrontierErrorCode_SystemError:
      throw SystemError(errorMessage);
    case FrontierErrorCode_NetworkProblem:
      throw NetworkProblem(errorMessage);
    case FrontierErrorCode_ProtocolError:
      throw ProtocolError(errorMessage);
    case FrontierErrorCode_UnknownError:
    default:
      throw FrontierException(errorMessage, errorCode);
  }
  throw FrontierException(errorMessage, errorCode);
}

} // namespace frontier



