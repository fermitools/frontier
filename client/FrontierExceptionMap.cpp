/*
 * Frontier exception mapper.
 *
 * $Id$
 *
 */
#
#include <string>
#include <iostream>
#include <exception>

#include "frontier_client/FrontierException.hpp"
#include "frontier_client/FrontierExceptionMap.hpp"

namespace frontier {

void FrontierExceptionMap::throwException(
  int errorCode, const std::string& errorMessage) {
  std::cout << "Here is my code..." << errorCode << std::endl;
  switch(errorCode) {
    case FrontierErrorCode_InvalidArgument:
      break;
    case FrontierErrorCode_MemoryAllocationFailed:
      break;
    case FrontierErrorCode_ConfigurationError:
      std::cout << "Here is my arror..." << std::endl;
      throw ConfigurationError(errorMessage);
    case FrontierErrorCode_SystemError:
      break;
    case FrontierErrorCode_Unknown:
      break;
    case FrontierErrorCode_NetworkProblem:
      break;
    case FrontierErrorCode_ProtocolError:
      break;
    case FrontierErrorCode_UndefinedError:
    default:
      throw FrontierException(errorMessage, errorCode);
  }
  throw FrontierException(errorMessage, errorCode);
}

} // namespace frontier



