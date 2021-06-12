/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
/** \file commands/locks/common.h
 * Common code used by different commands.
 */
#ifndef ZYPPER_COMMANDS_LOCKS_COMMON_H_INCLUDED
#define ZYPPER_COMMANDS_LOCKS_COMMON_H_INCLUDED

#include <zypp/PoolQuery.h>

#include "Zypper.h"

///////////////////////////////////////////////////////////////////
namespace locks
{
  /** Add/remove locks need to translate their cli args into PoolQueries in a common manner.
   */
  zypp::PoolQuery arg2query( Zypper & zypper, const std::string & arg_r, const std::set<zypp::ResKind> & kinds_r, const std::vector<std::string> & repos_r, const std::string & comment_r );

} // namespace locks
///////////////////////////////////////////////////////////////////
#endif // ZYPPER_COMMANDS_LOCKS_COMMON_H_INCLUDED
