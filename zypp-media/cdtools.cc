/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include "cdtools.h"

extern "C"
{
#include <sys/ioctl.h>
#include <linux/cdrom.h>
}

#include <fcntl.h>
#include <unistd.h>
#include <zypp-core/base/LogControl.h>
#include <zypp-core/ExternalProgram.h>


/*
** If defined to the full path of the eject utility,
** it will be used additionally to the eject-ioctl.
*/
#define EJECT_TOOL_PATH "/bin/eject"


namespace zypp::media {

  bool CDTools::openTray(const std::string &device_r)
  {
    int fd = ::open( device_r.c_str(), O_RDONLY|O_NONBLOCK|O_CLOEXEC );
    int res = -1;

    if ( fd != -1)
    {
      res = ::ioctl( fd, CDROMEJECT );
      ::close( fd );
    }

    if ( res )
    {
      if( fd == -1)
      {
        WAR << "Unable to open '" << device_r
            << "' (" << ::strerror( errno ) << ")" << std::endl;
      }
      else
      {
        WAR << "Eject " << device_r
            << " failed (" << ::strerror( errno ) << ")" << std::endl;
      }

#if defined(EJECT_TOOL_PATH)
      DBG << "Try to eject " << device_r << " using "
          << EJECT_TOOL_PATH << " utility" << std::endl;

      const char *cmd[3];
      cmd[0] = EJECT_TOOL_PATH;
      cmd[1] = device_r.c_str();
      cmd[2] = NULL;
      ExternalProgram eject(cmd, ExternalProgram::Stderr_To_Stdout);

      for(std::string out( eject.receiveLine());
            out.length(); out = eject.receiveLine())
      {
        DBG << " " << out;
      }

      if(eject.close() != 0)
      {
        WAR << "Eject of " << device_r << " failed." << std::endl;
        return false;
      }
#else
      return false;
#endif
    }
    MIL << "Eject of " << device_r << " successful." << std::endl;
    return true;
  }

  bool CDTools::closeTray(const std::string &device_r)
  {
    int fd = ::open( device_r.c_str(), O_RDONLY|O_NONBLOCK|O_CLOEXEC );
    if ( fd == -1 ) {
      WAR << "Unable to open '" << device_r << "' (" << ::strerror( errno ) << ")" << std::endl;
      return false;
    }
    int res = ::ioctl( fd, CDROMCLOSETRAY );
    ::close( fd );
    if ( res ) {
      WAR << "Close tray " << device_r << " failed (" << ::strerror( errno ) << ")" << std::endl;
      return false;
    }
    DBG << "Close tray " << device_r << std::endl;
    return true;
  }

}
