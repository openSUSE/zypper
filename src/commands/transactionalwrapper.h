/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_TRANSACTIONALWRAPPER_H_INCLUDED
#define ZYPPER_COMMANDS_TRANSACTIONALWRAPPER_H_INCLUDED

#include <string>
#include "commands/basecommand.h"
#include <zypp-core/base/Exception.h>
#include <zypp/Pathname.h>

class Zypper;

struct TransactionalWrapperException : public zypp::Exception
{
  TransactionalWrapperException() {}
  TransactionalWrapperException( std::string msg_r )
  : zypp::Exception( std::move(msg_r) )
  {}
};

/** Whether system (at root_r) is readonly. */
bool readOnlyRootAt( const zypp::Pathname & root_r );

class TransactionalWrapper
{
public:
  /** Whether system (at /) is a transactional system. */
  static bool isTransactionalSystem();

  /** Whether system (at /) has transactional-wrapper installed. */
  static bool hasTransactionalWrapper();

  /** Use can be disabled by setting ZYPPER_USE_TRANSACTIONAL_WRAPPER=0 */
  static bool mayUseTransactionalWrapper();

  /** Run transactional-wrapper (re-runs the command in a RW context). */
  static int run();
};

#endif
