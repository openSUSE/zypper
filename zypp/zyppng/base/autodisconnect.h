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
  /*!
   * This can be used to scope a signal/slot connection.
   * Basically just like \sa zypp::AutoDispose
   */
  struct AutoDisconnect
  {
    AutoDisconnect( connection &&conn ) : _conn ( std::move(conn) ) {}
    AutoDisconnect( AutoDisconnect &&other ) : _conn ( std::move(other._conn) ) {}

    AutoDisconnect( const AutoDisconnect &other ) = delete;
    AutoDisconnect & operator=( const AutoDisconnect & ) = delete;

    ~AutoDisconnect( ) { _conn.disconnect(); }
    connection _conn;
  };

}
