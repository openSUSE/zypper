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
#ifndef ZYPPNG_MEDIA_NETWORK_AUTHDATA_H_INCLUDED
#define ZYPPNG_MEDIA_NETWORK_AUTHDATA_H_INCLUDED

#include <zypp-curl/auth/CurlAuthData>

namespace zyppng {

using AuthData = zypp::media::AuthData;
using AuthData_Ptr = zypp::media::AuthData_Ptr;

using NetworkAuthData = zypp::media::CurlAuthData;
using NetworkAuthData_Ptr = zypp::media::CurlAuthData_Ptr;

}




#endif
