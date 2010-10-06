/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/media/UrlResolverPlugin.cc
 *
*/
#include <iostream>
#include "zypp/base/Logger.h"
#include "zypp/media/UrlResolverPlugin.h"
#include "zypp/media/MediaException.h"
#include "zypp/PluginScript.h"
#include "zypp/ZConfig.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace media
  { /////////////////////////////////////////////////////////////////

    /** UrlResolverPlugin implementation. */
    struct UrlResolverPlugin::Impl
    {


    };
    ///////////////////////////////////////////////////////////////////

    Url UrlResolverPlugin::resolveUrl(const Url & o_url, HeaderList &headers)
    {
        if (o_url.getScheme() != "plugin")
            return o_url;        
        
        Url url(o_url);
        std::string name = url.getPathName();
        Pathname plugin_path = (ZConfig::instance().pluginsPath()/"urlresolver")/name;    
        if (PathInfo(plugin_path).isExist()) {
            PluginScript scr;
            scr.open(plugin_path);
            // send frame to plugin
            PluginFrame f("RESOLVEURL");

            url::ParamMap params = url.getQueryStringMap();
            url::ParamMap::const_iterator param_it;
            for( param_it = params.begin();
                 param_it != params.end();
                 ++param_it)
                f.setHeader(param_it->first, param_it->second);
            
            scr.send(f);

            PluginFrame r(scr.receive());
            if (r.command() == "RESOLVEDURL") {
                // now set
                url = Url(r.body());
                PluginFrame::HeaderListIterator it;
                
                for (it = r.headerBegin();
                     it != r.headerEnd();
                     ++it) {
                    std::pair<std::string, std::string> values(*it);
                    // curl resets headers that are empty, so we use a workaround
                    if (values.second.empty()) {
                        values.second = "\nX-libcurl-Empty-Header-Workaround: *";
                    }                    
                    headers.insert(values);                    
                }
            }
            else if (r.command() == "ERROR") {
                ZYPP_THROW(MediaException(r.body()));
            }            
        }
        return url;        
    }

    /** \relates UrlResolverPlugin::Impl Stream output */
    inline std::ostream & operator<<( std::ostream & str, const UrlResolverPlugin::Impl & obj )
    {
      return str << "UrlResolverPlugin::Impl";
    }

    UrlResolverPlugin::~UrlResolverPlugin()
    {}

    std::ostream & operator<<( std::ostream & str, const UrlResolverPlugin & obj )
    {
      return str << *obj._pimpl;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace media
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
