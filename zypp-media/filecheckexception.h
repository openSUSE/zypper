/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_MEDIA_FILECHECKEXCEPTION_H
#define ZYPP_MEDIA_FILECHECKEXCEPTION_H

#include <zypp-core/base/Exception.h>

namespace zypp {

  class FileCheckException : public Exception
  {
  public:
    FileCheckException(const std::string &msg)
      : Exception(msg)
    {}
  };

  class CheckSumCheckException : public FileCheckException
  {
  public:
    CheckSumCheckException(const std::string &msg)
      : FileCheckException(msg)
    {}
  };

  class SignatureCheckException : public FileCheckException
  {
  public:
    SignatureCheckException(const std::string &msg)
      : FileCheckException(msg)
    {}
  };

}

#endif // ZYPP_MEDIA_FILECHECKEXCEPTION_H
