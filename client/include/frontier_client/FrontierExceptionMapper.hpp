/*
 * frontier client C++ API exception mapper header
 *
 * Author: Sinisa Vaseli
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


