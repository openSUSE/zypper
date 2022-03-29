/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\----------------------------------------------------------------------/
*
* This file contains private API, this might break at any time between releases.
* You have been warned!
*
*/
#ifndef ZYPP_MEDIA_PROVIDE_FWD_H_INCLUDED
#define ZYPP_MEDIA_PROVIDE_FWD_H_INCLUDED

#include <memory>
#include <zypp-core/zyppng/base/zyppglobal.h>

namespace zyppng {
  ZYPP_FWD_DECL_TYPE_WITH_REFS (Provide);
  ZYPP_FWD_DECL_TYPE_WITH_REFS (ProvideMediaSpec);
  ZYPP_FWD_DECL_TYPE_WITH_REFS (ProvideFileSpec);
  ZYPP_FWD_DECL_TYPE_WITH_REFS (ProvideItem);
  ZYPP_FWD_DECL_TYPE_WITH_REFS (ProvideRequest);
  ZYPP_FWD_DECL_TYPE_WITH_REFS (MediaDataVerifier);
  ZYPP_FWD_DECL_TYPE_WITH_REFS (ProvideStatus);
  class HeaderValueMap;
  class ProvideMediaHandle;
}

#endif
