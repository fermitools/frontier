/*
 * Frontier exceptions.
 *
 * $Id$
 *
 */
#
#ifndef FRONTIER_EXCEPTION_HPP
#define FRONTIER_EXCEPTION_HPP

#include <string>
#include <iostream>
#include <exception>

#include "frontier_client/frontier_error.h"

namespace frontier {

  static const int FrontierErrorCode_OK = FRONTIER_OK;
  static const int FrontierErrorCode_InvalidArgument = FRONTIER_EIARG;
  static const int FrontierErrorCode_MemoryAllocationFailed = FRONTIER_EMEM;
  static const int FrontierErrorCode_ConfigurationError = FRONTIER_ECFG;
  static const int FrontierErrorCode_SystemError = FRONTIER_ESYS;
  static const int FrontierErrorCode_Unknown = FRONTIER_EUNKNOWN;
  static const int FrontierErrorCode_NetworkProblem = FRONTIER_ENETWORK;
  static const int FrontierErrorCode_ProtocolError = FRONTIER_EPROTO;
  static const int FrontierErrorCode_UndefinedError = FRONTIER_EUNDEFINED;

  // Generic exception.
  class FrontierException : public std::exception {
  private:
    std::string _error;
    int _errorCode;
  public:
    FrontierException() : _error(), _errorCode(FrontierErrorCode_UndefinedError) { }
    ~FrontierException() throw() { }
    FrontierException(const std::string& err = "", int errorCode = FrontierErrorCode_UndefinedError) : _error(err), _errorCode(errorCode) { }
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
    RuntimeError(const std::string& err = "", int errorCode = FrontierErrorCode_UndefinedError) : FrontierException(err, errorCode) { }
  };

  // Configuration error.
  class ConfigurationError : public FrontierException {
  public:
    ConfigurationError(const std::string& err = "") : FrontierException(err, FrontierErrorCode_ConfigurationError) { }
  };

  // Logic error.
  class LogicError : public FrontierException {
  public:
    LogicError(const std::string& err = "") : FrontierException(err) { }
  };

} // namespace frontier

#endif // ifndef FRONTIER_EXCEPTION


