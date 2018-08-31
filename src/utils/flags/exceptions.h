/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPP_FLAGS_EXCEPTIONS_H_INCLUDED
#define ZYPP_FLAGS_EXCEPTIONS_H_INCLUDED

#include <zypp/base/Exception.h>

namespace zypp {
namespace ZyppFlags {

  class ZyppFlagsException : public zypp::Exception
  {
  public:
    ZyppFlagsException(const std::string &msg);
  };

  /**
   * The parser encountered a unknown flag
   */
  class UnknownFlagException : public ZyppFlagsException
  {
  public:
    UnknownFlagException(const std::string &flag);
  };

  /**
   * The value given on the commandline was not compatible with the flag
   */
  class InvalidValueException : public ZyppFlagsException
  {
  public:
    InvalidValueException(const std::string &flag, const std::string &invalidValue, const std::string &reason);
  };

  /**
   * There was no argument given for a flag, but it was required
   */
  class MissingArgumentException : public ZyppFlagsException
  {
  public:
    MissingArgumentException(const std::string &flag);
  };

  /**
   * The flag was seen multiple times on the commandline but did not allow to do so
   */
  class FlagRepeatedException : public ZyppFlagsException
  {
  public:
    FlagRepeatedException ( const std::string &flag );
  };
}}


#endif
