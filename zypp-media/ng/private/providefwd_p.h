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
#ifndef ZYPP_MEDIA_PRIVATE_PROVIDE_FWD_P_H_INCLUDED
#define ZYPP_MEDIA_PRIVATE_PROVIDE_FWD_P_H_INCLUDED

#include <zypp-media/ng/ProvideFwd>
namespace zyppng {
  ZYPP_FWD_DECL_TYPE_WITH_REFS(ProvideQueue);
  ZYPP_FWD_DECL_TYPE_WITH_REFS(ProvideWorker);
  ZYPP_FWD_DECL_TYPE_WITH_REFS(ProvideFileItem);
  ZYPP_FWD_DECL_TYPE_WITH_REFS(AttachMediaItem);
  ZYPP_FWD_DECL_TYPE_WITH_REFS(DetachMediaItem);
  ZYPP_FWD_DECL_TYPE_WITH_REFS(ProvideRequest);

  class ProvideMessage;

  template< typename T >
  class ProvidePromise;
  template< typename T >
  using ProvidePromiseRef = std::shared_ptr<ProvidePromise<T>>;
  template< typename T >
  using ProvidePromiseWeakRef = std::weak_ptr<ProvidePromise<T>>;
}

#endif
