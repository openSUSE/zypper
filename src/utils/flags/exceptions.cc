/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "exceptions.h"

#include <zypp/base/String.h>

#include "main.h"

namespace zypp {
namespace ZyppFlags {

ZyppFlagsException::ZyppFlagsException(const std::string &msg)
  : Exception(msg)
{}

UnknownFlagException::UnknownFlagException(const std::string &flag)
 : ZyppFlagsException( str::Format(_("The flag %1% is not known.")) % flag )
{

}

InvalidValueException::InvalidValueException(const std::string &flag, const std::string &invalidValue, const std::string &reason)
  : ZyppFlagsException( str::Format(_("The flag %1% is not compatible with argument %2% (%3%)."))  % flag % invalidValue % reason )
{

}

MissingArgumentException::MissingArgumentException(const std::string &flag)
  : ZyppFlagsException( str::Format( _("The flag %1% requires a argument.") ) % flag )
{

}

FlagRepeatedException::FlagRepeatedException(const std::string &flag)
  : ZyppFlagsException( str::Format( _("The flag %1% can only be used once.") ) % flag )
{

}

ConflictingFlagsException::ConflictingFlagsException(const std::string &flag, const std::string &flag2)
  : ZyppFlagsException( str::Format(  _("%s used together with %s, which contradict each other.") ) % flag % flag2 )
{

}

}}
