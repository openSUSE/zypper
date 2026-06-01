/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "transactionalwrapper.h"

#include <cerrno>
#include <cstring>
#include <unistd.h>

#include <zypp-core/base/String.h>

#include "main.h"

int TransactionalWrapper::run( std::string &err_r ) const
{
  const char * transactionalWrapper = "/usr/sbin/transactional-wrapper";
  char *const argv[] = {
    const_cast<char *>( transactionalWrapper ),
    const_cast<char *>( "zypper" ),
    nullptr
  };

  execv( transactionalWrapper, argv );
  err_r = str::Format(_("Failed to execute %1% (%2%).")) % transactionalWrapper % strerror(errno);
  return ZYPPER_EXIT_ERR_PRIVILEGES;
}
