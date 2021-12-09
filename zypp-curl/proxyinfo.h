/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp-curl/ProxyInfo
 *
*/
#ifndef ZYPP_CURL_PROXYINFO_H_INCLUDED
#define ZYPP_CURL_PROXYINFO_H_INCLUDED

#include <string>
#include <list>

#include <zypp-core/base/PtrTypes.h>

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

#endif // ZYPP_CURL_PROXYINFO_H_INCLUDED
