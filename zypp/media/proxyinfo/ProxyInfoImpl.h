/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/proxyinfo/ProxyInfoImpl.h
 *
*/
#ifndef ZYPP_MEDIA_PROXYINFO_PROXYINFOIMPL_H
#define ZYPP_MEDIA_PROXYINFO_PROXYINFOIMPL_H

#include <string>
#include <list>

#include "zypp/Url.h"
#include "zypp/base/String.h"
#include "zypp/media/ProxyInfo.h"

namespace zypp {
  namespace media {

    struct ProxyInfo::Impl
    {
      /** Ctor */
      Impl()
      {}

      /** Dtor */
      virtual ~Impl()
      {}

    public:
      /**  */
      virtual bool enabled() const = 0;
      /**  */
      virtual std::string proxy(const Url & url_r) const = 0;
      /**  */
      virtual ProxyInfo::NoProxyList noProxy() const = 0;
      /**  */
      virtual ProxyInfo::NoProxyIterator noProxyBegin() const = 0;
      /**  */
      virtual ProxyInfo::NoProxyIterator noProxyEnd() const = 0;

      /** Return \c true if  \ref enabled and \a url_r does not match \ref noProxy. */
      bool useProxyFor( const Url & url_r ) const
      {
        if ( ! enabled() && proxy( url_r ).empty() )
          return false;

        ProxyInfo::NoProxyList noproxy( noProxy() );
        if ( noproxy.size() == 1 && noproxy.front() == "*" )
          return false; // just an asterisk disables all.

        // No proxy: Either an exact match, or the previous character
        // is a '.', so host is within the same domain.
        // A leading '.' in the pattern is ignored. Some implementations
        // need '.foo.ba' to prevent 'foo.ba' from matching 'xfoo.ba'.
        std::string host( str::toLower( url_r.getHost() ) );
        for_( it, noproxy.begin(), noproxy.end() )
        {
          std::string pattern( str::toLower( (*it)[0] == '.' ? it->c_str() + 1 : it->c_str() ) );
          if ( str::hasSuffix( host, pattern )
               && ( host.size() == pattern.size()
                    || host[host.size()-pattern.size()-1] == '.' ) )
            return false;
        }
        return true;
      }

    public:
      /** Default Impl: empty sets. */
      static shared_ptr<Impl> _nullimpl;
    };


///////////////////////////////////////////////////////////////////

  } // namespace media
} // namespace zypp

#endif // ZYPP_MEDIA_PROXYINFO_PROXYINFOIMPL_H
