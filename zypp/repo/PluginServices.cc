/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
#include <iostream>
#include <sstream>
#include "zypp/base/Logger.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/String.h"
#include "zypp/base/InputStream.h"
#include "zypp/base/UserRequestException.h"

#include "zypp/repo/PluginServices.h"
#include "zypp/ServiceInfo.h"
#include "zypp/RepoInfo.h"
#include "zypp/PathInfo.h"

using std::endl;
using std::stringstream;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace repo
  { /////////////////////////////////////////////////////////////////

    class PluginServices::Impl
    {
    public:
      static void loadServices( const Pathname &path,
          const PluginServices::ProcessService &callback );
    };

    void PluginServices::Impl::loadServices( const Pathname &path,
                                  const PluginServices::ProcessService & callback/*,
                                  const ProgressData::ReceiverFnc &progress*/ )
    {
      std::list<Pathname> entries;
      if (PathInfo(path).isExist())
      {
        if ( filesystem::readdir( entries, path, false ) != 0 )
        {
          // TranslatorExplanation '%s' is a pathname
            ZYPP_THROW(Exception(str::form(_("Failed to read directory '%s'"), path.c_str())));
        }

        //str::regex allowedServiceExt("^\\.service(_[0-9]+)?$");
        for_(it, entries.begin(), entries.end() )
        {
          ServiceInfo service_info;
          service_info.setAlias((*it).basename());
          Url url;
          url.setPathName((*it).asString());
          url.setScheme("file");
          service_info.setUrl(url);
          service_info.setType(ServiceType::PLUGIN);
          service_info.setAutorefresh( true );
	  DBG << "Plugin Service: " << service_info << endl;
          callback(service_info);
        }

      }
    }

    PluginServices::PluginServices( const Pathname &path,
                                  const ProcessService & callback/*,
                                  const ProgressData::ReceiverFnc &progress */)
    {
      Impl::loadServices(path, callback/*, progress*/);
    }

    PluginServices::~PluginServices()
    {}

    std::ostream & operator<<( std::ostream & str, const PluginServices & obj )
    {
      return str;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace repo
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
