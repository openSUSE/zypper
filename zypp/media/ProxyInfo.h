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
#include <map>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"

namespace zypp {
  namespace media {

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ProxyInfo
    class ProxyInfo : public base::ReferenceCounted, private base::NonCopyable
    {
    public:
      typedef intrusive_ptr<ProxyInfo> Ptr;
      typedef intrusive_ptr<ProxyInfo> constPtr;
      /** Implementation */
      struct Impl;
      ProxyInfo();
      ProxyInfo(RW_pointer<Impl> impl);
      bool enabled();
      std::string http();
      std::string ftp();
      std::string https();
      std::list<std::string> noProxy();

    private:
      /** Pointer to implementation */
      RW_pointer<Impl> _pimpl;
    };

    struct ProxyInfo::Impl
    {
      /** Ctor */
      Impl()
      : _enabled( false )
      , _http( "" )
      , _ftp( "" )
      , _https( "" )
      , _no_proxy()
      {}
      /** Ctor */
      Impl( const bool enabled,
	    const std::string & http_r,
	    const std::string & ftp_r,
	    const std::string & https_r,
	    const std::list<std::string> & no_proxy_r )
      : _enabled( enabled )
      , _http( http_r )
      , _ftp( ftp_r )
      , _https( https_r )
      , _no_proxy( no_proxy_r )
      {}
  
    public:
      /**  */
      const bool & enabled() const
      { return _enabled; }
      /**  */
      const std::string & http() const
      { return _http; }
      /**  */
      const std::string & ftp() const
      { return _ftp; }
      /**  */
      const std::string & https() const
      { return _https; }
      /**  */
      const std::list<std::string> & noProxy() const
      { return _no_proxy; }
  
     protected:
      bool _enabled;
      std::string _http;
      std::string _ftp;
      std::string _https;
      std::list<std::string> _no_proxy;
    public:
      /** Default Impl: empty sets. */
      static shared_ptr<Impl> _nullimpl;
    };

    class ProxyInfo_sysconfig : public ProxyInfo::Impl
    {
    public:
      ProxyInfo_sysconfig(const Pathname & path);
    };

    namespace proxyinfo {
      std::map<std::string,std::string> sysconfigRead(const Pathname & _path);
    } // namespace proxyinfo

///////////////////////////////////////////////////////////////////

  } // namespace media
} // namespace zypp

#endif // ZYPP_MEDIA_PROXYINFO_H
