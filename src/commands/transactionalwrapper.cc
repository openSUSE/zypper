/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <sys/vfs.h>
#include <sys/statvfs.h>
#include <linux/magic.h>

#include <string>

#include <zypp/PathInfo.h>

#include "transactionalwrapper.h"
#include "subcommand.h"
#include "Zypper.h"

#ifdef TRUE_IF_FAKE_TRANSACTIONAL
#undef TRUE_IF_FAKE_TRANSACTIONAL
#endif
#if 0
#define TRUE_IF_FAKE_TRANSACTIONAL return true;
#else
#define TRUE_IF_FAKE_TRANSACTIONAL
#endif

namespace {
  const std::string tsIndicatorProg = "/usr/sbin/transactional-update";
  const std::string tsWrapperProg   = "/usr/sbin/transactional-wrapper";
} // namespace

bool readOnlyRootAt( const Pathname & root_r )
{
  TRUE_IF_FAKE_TRANSACTIONAL;

  struct statfs fsinfo;
  memset( &fsinfo, 0, sizeof(struct statfs) );
  int res = 0;
  do {
    res = statfs( root_r.c_str(), &fsinfo );
  } while ( res == -1 && errno == EINTR );
  return res == 0 && fsinfo.f_flags & ST_RDONLY;
}

bool TransactionalWrapper::isTransactionalSystem()
{
  TRUE_IF_FAKE_TRANSACTIONAL;

  struct statfs fsinfo;
  memset( &fsinfo, 0, sizeof(struct statfs) );
  int res = 0;
  do {
    res = statfs( "/", &fsinfo );
  } while ( res == -1 && errno == EINTR );

  if ( res == 0 && fsinfo.f_flags & ST_RDONLY && fsinfo.f_type == BTRFS_SUPER_MAGIC ) {
    PathInfo pi { tsIndicatorProg };
    return pi.isFile() && pi.userMayRX();
  }
  return false;
}

bool TransactionalWrapper::hasTransactionalWrapper()
{
  PathInfo pi { tsWrapperProg };
  return pi.isFile() && pi.userMayRX();
}

bool TransactionalWrapper::mayUseTransactionalWrapper()
{
  const char * env = ::getenv( "ZYPPER_USE_TRANSACTIONAL_WRAPPER" );
  if ( env )
    pDBG( "ZYPPER_USE_TRANSACTIONAL_WRAPPER=", env );
  return not env || str::strToBool( env, true );
}

int TransactionalWrapper::run()
{
  pMIL( "TransactionalWrapper::run..." );
  SubCmd::Arglist cmd { { tsWrapperProg, "--success-exit-codes", "102,103" } };
  return SubCmd::doRunAsSubcommand( Zypper::instance(), std::move(cmd) );
}
