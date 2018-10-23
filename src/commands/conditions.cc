/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "conditions.h"
#include "global-settings.h"
#include "Zypper.h"

#include <sys/vfs.h>
#include <sys/statvfs.h>
#include <linux/magic.h>

int NeedsRootCondition::check(std::string &err)
{
  if ( geteuid() != 0 && !Zypper::instance().globalOpts().changedRoot )
  {
    err = _("Root privileges are required to run this command.");
    return ZYPPER_EXIT_ERR_PRIVILEGES;
  }
  return ZYPPER_EXIT_OK;
}

int NeedsWritableRoot::check(std::string &err_r)
{
  Zypper &zypper = Zypper::instance();
  const GlobalOptions &gopts = zypper.globalOpts();

  if ( DryRunSettings::instance().isEnabled() )
    return ZYPPER_EXIT_OK;

  struct statfs fsinfo;
  memset( &fsinfo, 0, sizeof(struct statfs) );

  int errCode = 0;
  do {
    errCode = statfs( gopts.root_dir.c_str(), &fsinfo );
  } while ( errCode == -1 && errno == EINTR );

  if ( !errCode ) {
    if ( fsinfo.f_flags & ST_RDONLY ) {

      bool isTransactionalServer = ( fsinfo.f_type == BTRFS_SUPER_MAGIC && PathInfo( "/usr/sbin/transactional-update" ).isFile() );

      if ( isTransactionalServer && !gopts.changedRoot ) {
        err_r = _("This is a transactional-server, please use transactional-update to update or modify the system.");
      } else {
        err_r = _("The target filesystem is mounted as read-only. Please make sure the target filesystem is writeable.");
      }

      ERR << err_r << endl;
      return ZYPPER_EXIT_ERR_PRIVILEGES;
    }
  } else {
    WAR << "Checking if " << gopts.root_dir << " is mounted read only failed with errno : " << errno << std::endl;
  }
  return ZYPPER_EXIT_OK;
}
