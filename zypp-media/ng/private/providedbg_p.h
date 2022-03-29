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
#ifndef ZYPP_MEDIA_NG_PROVIDEDBG_P_H_INCLUDED
#define ZYPP_MEDIA_NG_PROVIDEDBG_P_H_INCLUDED

#include <zypp-core/base/LogControl.h>

L_ENV_CONSTR_FWD_DECLARE_FUNC(ZYPP_MEDIA_PROVIDER_DEBUG)

#ifdef ZYPP_BASE_LOGGER_LOGGROUP
#undef ZYPP_BASE_LOGGER_LOGGROUP
#endif

#define ZYPP_BASE_LOGGER_LOGGROUP "ZYPP_MEDIA_PROVIDE"

namespace zyppng {
  inline bool provideDebugEnabled() {
    return zypp::log::has_env_constr_ZYPP_MEDIA_PROVIDER_DEBUG();
  }
}

#define XXX_PRV if( zyppng::provideDebugEnabled() ) XXX
#define DBG_PRV if( zyppng::provideDebugEnabled() ) DBG
#define MIL_PRV if( zyppng::provideDebugEnabled() ) MIL
#define WAR_PRV if( zyppng::provideDebugEnabled() ) WAR
#define ERR_PRV if( zyppng::provideDebugEnabled() ) ERR
#define SEC_PRV if( zyppng::provideDebugEnabled() ) SEC
#define INT_PRV if( zyppng::provideDebugEnabled() ) INT
#define USR_PRV if( zyppng::provideDebugEnabled() ) USR


#endif // ZYPP_MEDIA_NG_PROVIDEDBG_P_H_INCLUDED
