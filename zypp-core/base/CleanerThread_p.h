/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/CleanerThread_p.h
 * This file contains private API, it will change without notice.
 * You have been warned.
*/

#include <unistd.h>

#include <zypp-core/Globals.h>

namespace zypp
{

  class ZYPP_LOCAL CleanerThread
  {
    public:
      CleanerThread() = delete;
      static void watchPID ( pid_t pid_r );
  };

}
