/*
 * frontier client C++ exception mapper
 *
 * Author: Sinisa Veseli
 *
 * $Id$
 *
 * Copyright (c) 2009, FERMI NATIONAL ACCELERATOR LABORATORY
 * All rights reserved. 
 *
 * For details of the Fermitools (BSD) license see Fermilab-2009.txt or
 *  http://fermitools.fnal.gov/about/terms.html
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
    case FrontierErrorCode_ServerError:
      throw ServerError(errorMessage);
    case FrontierErrorCode_UnknownError:
    default:
      throw FrontierException(errorMessage, errorCode);
  }
  throw FrontierException(errorMessage, errorCode);
}

} // namespace frontier



