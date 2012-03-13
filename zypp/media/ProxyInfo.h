/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/ProxyInfo.h
 *
*/
#ifndef ZYPP_MEDIA_PROXYINFO_H
#define ZYPP_MEDIA_PROXYINFO_H

#include <string>
#include <list>

#include "zypp/base/PtrTypes.h"

namespace zypp {

  class Url;

  namespace media {

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ProxyInfo
    class ProxyInfo
    {
    public:
      typedef intrusive_ptr<ProxyInfo> Ptr;
      typedef intrusive_ptr<ProxyInfo> constPtr;
      typedef std::list<std::string> NoProxyList;
      typedef std::list<std::string>::const_iterator NoProxyIterator;

      /** Implementation */
      struct Impl;
      typedef shared_ptr<Impl> ImplPtr;

      /** Default Ctor: guess the best available implementation. */
      ProxyInfo();
      /** Ctor taking a specific implementation. */
      ProxyInfo( ProxyInfo::ImplPtr pimpl_r );

      bool enabled() const;
      std::string proxy(const Url & url) const;
      NoProxyList noProxy() const;
      NoProxyIterator noProxyBegin() const;
      NoProxyIterator noProxyEnd() const;

      /** Return \c true if  \ref enabled and \a url_r does not match \ref noProxy. */
      bool useProxyFor( const Url & url_r ) const;

    private:
      /** Pointer to implementation */
      RW_pointer<Impl> _pimpl;
    };


///////////////////////////////////////////////////////////////////

  } // namespace media
} // namespace zypp

#endif // ZYPP_MEDIA_PROXYINFO_H
