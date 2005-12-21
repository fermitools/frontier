/*
 * Frontier exception mapper.
 *
 * $Id$
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

  class FrontierExceptionMap {
  private:
    FrontierExceptionMap() {}
  public:
    static void throwException(int errorCode, const std::string& errorMessage);
  };

} // namespace frontier

#endif // ifndef FRONTIER_EXCEPTION_MAP_HPP


