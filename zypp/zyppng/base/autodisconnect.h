/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
----------------------------------------------------------------------/
*
* This file contains private API, this might break at any time between releases.
* You have been warned!
*
*/

#include <zypp/zyppng/base/Signals>

namespace zyppng
{
  struct AutoDisconnect
  {
    AutoDisconnect( sigc::connection &&conn ) : _conn ( std::move(conn) ) {}
    ~AutoDisconnect( ) { _conn.disconnect(); }
    sigc::connection _conn;
  };

}
