/*
 * frontier client C++ API exceptions header
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
#ifndef FRONTIER_EXCEPTION_HPP
#define FRONTIER_EXCEPTION_HPP

#include <string>
#include <iostream>
#include <exception>

extern "C" {
#include "frontier_client/frontier_error.h"
}



namespace frontier {

  static const int FrontierErrorCode_OK = FRONTIER_OK;
  static const int FrontierErrorCode_InvalidArgument = FRONTIER_EIARG;
  static const int FrontierErrorCode_MemoryAllocationFailed = FRONTIER_EMEM;
  static const int FrontierErrorCode_ConfigurationError = FRONTIER_ECFG;
  static const int FrontierErrorCode_SystemError = FRONTIER_ESYS;
  static const int FrontierErrorCode_UnknownError = FRONTIER_EUNKNOWN;
  static const int FrontierErrorCode_NetworkProblem = FRONTIER_ENETWORK;
  static const int FrontierErrorCode_ProtocolError = FRONTIER_EPROTO;

  // Generic exception.
  class FrontierException : public std::exception {
  private:
    std::string _error;
    int _errorCode;
  public:
    virtual ~FrontierException() throw() { }
    FrontierException(const std::string& err = "", int errorCode = FrontierErrorCode_UnknownError) : _error(err), _errorCode(errorCode) { 
      std::string additionalInfo(frontier_getErrorMsg());
      if(additionalInfo != std::string("")) {
        _error = _error + " (Additional Information: " + additionalInfo + ")";
      }
    }
    virtual std::string getError() const throw() { return error(); }
    virtual int getErrorCode() const throw() { return _errorCode; }
    virtual std::string error() const throw() { return _error; }
    virtual const char* what() const throw() { return _error.c_str(); }
  };

  // Output.
  inline std::ostream& operator<<(std::ostream& os,
                                  const FrontierException& ex) {
    os << ex.error();
    return os;
  }

  // Runtime error.
  class RuntimeError : public FrontierException {
  public:
    RuntimeError(const std::string& err = "", int errorCode = FrontierErrorCode_UnknownError) : FrontierException(err, errorCode) { }
  };

  // Configuration error.
  class ConfigurationError : public FrontierException {
  public:
    ConfigurationError(const std::string& err = "") : FrontierException(err, FrontierErrorCode_ConfigurationError) { }
  };

  // Logic error.
  class LogicError : public FrontierException {
  public:
    LogicError(const std::string& err = "", int errorCode = FrontierErrorCode_UnknownError) : FrontierException(err, errorCode) { }
  };

  // Invalid argument.
  class InvalidArgument : public LogicError {
  public:
    InvalidArgument(const std::string& err = "") : LogicError(err, FrontierErrorCode_InvalidArgument) { }
  };

  // System error.
  class SystemError : public FrontierException {
  public:
    SystemError(const std::string& err = "", int errorCode = FrontierErrorCode_SystemError) : FrontierException(err, errorCode) { }
  };

  // Memory allocation error.
  class MemoryAllocationFailed : public SystemError {
  public:
    MemoryAllocationFailed(const std::string& err = "") : SystemError(err, FrontierErrorCode_MemoryAllocationFailed) { }
  };

  // System error.
  class NetworkProblem : public FrontierException {
  public:
    NetworkProblem(const std::string& err = "") : FrontierException(err, FrontierErrorCode_NetworkProblem) { }
  };

  // Protocol error.
  class ProtocolError : public FrontierException {
  public:
    ProtocolError(const std::string& err = "") : FrontierException(err, FrontierErrorCode_ProtocolError) { }
  };

} // namespace frontier

#endif // ifndef FRONTIER_EXCEPTION


