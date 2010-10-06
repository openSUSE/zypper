/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/media/UrlResolverPlugin.h
 *
*/
#ifndef ZYPP_MEDIA_URLRESOLVERPLUGIN_H
#define ZYPP_MEDIA_URLRESOLVERPLUGIN_H

#include <iosfwd>
#include <map>
#include <string>

#include "zypp/base/PtrTypes.h"
#include "zypp/Url.h"
#include "zypp/PathInfo.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace media
  { /////////////////////////////////////////////////////////////////

    /** 
     *
     */
    class UrlResolverPlugin
    {
      friend std::ostream & operator<<( std::ostream & str, const UrlResolverPlugin & obj );

    public:

      class Impl;

      typedef std::multimap<std::string, std::string> HeaderList;

      /**
       * Resolves an url using the installed plugins
       * If no plugin is found the url is resolved as
       * its current value.
       *
       * Custom headers are inserted in the provided header list
       */
      static Url resolveUrl(const Url &url, HeaderList &headers);

    public:
      /** Dtor */
      ~UrlResolverPlugin();

    private:

      /** Default ctor */
      UrlResolverPlugin();

      /** Pointer to implementation */
      RW_pointer<Impl> _pimpl;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates UrlResolverPlugin Stream output */
    std::ostream & operator<<( std::ostream & str, const UrlResolverPlugin & obj );

    /////////////////////////////////////////////////////////////////
  } // namespace media
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_MEDIA_URLRESOLVERPLUGIN_H
