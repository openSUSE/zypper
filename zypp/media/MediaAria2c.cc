/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MediaAria2c.cc
 *
*/

#include <iostream>
#include <list>

#include "zypp/base/Logger.h"
#include "zypp/ExternalProgram.h"
#include "zypp/ProgressData.h"
#include "zypp/base/String.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/Sysconfig.h"
#include "zypp/base/Gettext.h"
#include "zypp/ZYppCallbacks.h"

#include "zypp/Edition.h"
#include "zypp/Target.h"
#include "zypp/ZYppFactory.h"

#include "zypp/media/MediaAria2c.h"
#include "zypp/media/proxyinfo/ProxyInfos.h"
#include "zypp/media/ProxyInfo.h"
#include "zypp/media/MediaUserAuth.h"
#include "zypp/media/MediaCurl.h"
#include "zypp/thread/Once.h"
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <boost/format.hpp>

#define  DETECT_DIR_INDEX       0
#define  CONNECT_TIMEOUT        60
#define  TRANSFER_TIMEOUT       60 * 3
#define  TRANSFER_TIMEOUT_MAX   60 * 60


using namespace std;
using namespace zypp::base;

namespace zypp
{
namespace media
{

Pathname MediaAria2c::_cookieFile = "/var/lib/YaST2/cookies";
Pathname MediaAria2c::_aria2cPath = "/usr/local/bin/aria2c";
std::string MediaAria2c::_aria2cVersion = "WE DON'T KNOW ARIA2C VERSION";

//check if aria2c is present in the system
bool
MediaAria2c::existsAria2cmd()
{
    const char* argv[] =
    {
      "which",
      "aria2c",
      NULL
    };

    ExternalProgram aria(argv, ExternalProgram::Stderr_To_Stdout);
    return ( aria.close() == 0 );
}

/**
 * comannd line for aria.
 * The argument list gets passed as reference
 * and it is filled.
 */
void fillAriaCmdLine( const Pathname &ariapath,
                      const string &ariaver,
                      const TransferSettings &s,
                      const Url &url,
                      const Pathname &destination,
                      ExternalProgram::Arguments &args )
{
    args.push_back(ariapath.c_str());
    args.push_back(str::form("--user-agent=%s", s.userAgentString().c_str()));
    args.push_back("--summary-interval=1");
    args.push_back("--follow-metalink=mem");
    args.push_back("--check-integrity=true");

    // only present in recent aria lets find out the aria version
    vector<string> fields;    
    // "aria2c version x.x"
    str::split( ariaver, std::back_inserter(fields));
    if ( fields.size() == 3 )
    {
        if ( Edition(fields[2]) >= Edition("1.1.2") )
            args.push_back( "--use-head=false");
    }
    
    if ( s.maxDownloadSpeed() > 0 )
        args.push_back(str::form("--max-download-limit=%ld", s.maxDownloadSpeed()));
    if ( s.minDownloadSpeed() > 0 )
        args.push_back(str::form("--lowest-speed-limit=%ld", s.minDownloadSpeed()));

    args.push_back(str::form("--max-tries=%ld", s.maxSilentTries()));

    if ( Edition(fields[2]) < Edition("1.2.0") )
        WAR << "aria2c is older than 1.2.0, some features may be disabled" << endl;
    
    // TODO make this one configurable
    args.push_back(str::form("--max-concurrent-downloads=%ld", s.maxConcurrentConnections()));

    // add the anonymous id.
    for ( TransferSettings::Headers::const_iterator it = s.headersBegin();
          it != s.headersEnd();
          ++it )
        args.push_back(str::form("--header=%s", it->c_str() ));
        
    args.push_back( str::form("--connect-timeout=%ld", s.timeout()));

    if ( s.username().empty() )
    {
        if ( url.getScheme() == "ftp" )
        {
            // set anonymous ftp
            args.push_back(str::form("--ftp-user=%s", "suseuser" ));
            args.push_back(str::form("--ftp-passwd=%s", VERSION ));

            string id = "yast2";
            id += VERSION;
            DBG << "Anonymous FTP identification: '" << id << "'" << endl;
        }
    }
    else
    {
        if ( url.getScheme() == "ftp" )
            args.push_back(str::form("--ftp-user=%s", s.username().c_str() ));
        else if ( url.getScheme() == "http" ||
                  url.getScheme() == "https" )
            args.push_back(str::form("--http-user=%s", s.username().c_str() ));
        
        if ( s.password().size() )
        {
            if ( url.getScheme() == "ftp" )
                args.push_back(str::form("--ftp-passwd=%s", s.password().c_str() ));
            else if ( url.getScheme() == "http" ||
                      url.getScheme() == "https" )
                args.push_back(str::form("--http-passwd=%s", s.password().c_str() ));
        }
    }
    
    if ( s.proxyEnabled() )
    {
        args.push_back(str::form("--http-proxy=%s", s.proxy().c_str() ));
        if ( ! s.proxyUsername().empty() )
        {
            args.push_back(str::form("--http-proxy-user=%s", s.proxyUsername().c_str() ));
            if ( ! s.proxyPassword().empty() )
                args.push_back(str::form("--http-proxy-passwd=%s", s.proxyPassword().c_str() ));
        }
    }

    if ( ! destination.empty() )
        args.push_back(str::form("--dir=%s", destination.c_str()));

    args.push_back(url.asString().c_str());
}

const char *const MediaAria2c::agentString()
{
  // we need to add the release and identifier to the
  // agent string.
  // The target could be not initialized, and then this information
  // is not available.
  Target_Ptr target = zypp::getZYpp()->getTarget();

  static const std::string _value(
    str::form(
       "ZYpp %s (%s) %s"
       , VERSION
       , MediaAria2c::_aria2cVersion.c_str()
       , target ? target->targetDistribution().c_str() : ""
    )
  );
  return _value.c_str();
}



MediaAria2c::MediaAria2c( const Url &      url_r,
                      const Pathname & attach_point_hint_r )
    : MediaCurl( url_r, attach_point_hint_r )
{
  MIL << "MediaAria2c::MediaAria2c(" << url_r << ", " << attach_point_hint_r << ")" << endl;

   //At this point, we initialize aria2c path
   _aria2cPath = Pathname( whereisAria2c().asString() );

   //Get aria2c version
   _aria2cVersion = getAria2cVersion();
}

void MediaAria2c::attachTo (bool next)
{
  MediaCurl::attachTo(next);
  _settings.setUserAgentString(agentString());
}

bool
MediaAria2c::checkAttachPoint(const Pathname &apoint) const
{
    return MediaCurl::checkAttachPoint( apoint );
}

void MediaAria2c::disconnectFrom()
{
    MediaCurl::disconnectFrom();
}

void MediaAria2c::releaseFrom( const std::string & ejectDev )
{
  MediaCurl::releaseFrom(ejectDev);
}

static Url getFileUrl(const Url & url, const Pathname & filename)
{
  Url newurl(url);
  string path = url.getPathName();
  if ( !path.empty() && path != "/" && *path.rbegin() == '/' &&
       filename.absolute() )
  {
    // If url has a path with trailing slash, remove the leading slash from
    // the absolute file name
    path += filename.asString().substr( 1, filename.asString().size() - 1 );
  }
  else if ( filename.relative() )
  {
    // Add trailing slash to path, if not already there
    if (path.empty()) path = "/";
    else if (*path.rbegin() != '/' ) path += "/";
    // Remove "./" from begin of relative file name
    path += filename.asString().substr( 2, filename.asString().size() - 2 );
  }
  else
  {
    path += filename.asString();
  }

  newurl.setPathName(path);
  return newurl;
}

void MediaAria2c::getFile( const Pathname & filename ) const
{
    // Use absolute file name to prevent access of files outside of the
    // hierarchy below the attach point.
    getFileCopy(filename, localPath(filename).absolutename());
}

void MediaAria2c::getFileCopy( const Pathname & filename , const Pathname & target) const
{
  callback::SendReport<DownloadProgressReport> report;

  Url fileurl(getFileUrl(_url, filename));

  bool retry = false;

  ExternalProgram::Arguments args;

  fillAriaCmdLine(_aria2cPath, _aria2cVersion, _settings, fileurl, target.dirname(), args);
  
  do
  {
    try
    {
      report->start(_url, target.asString() );

      ExternalProgram aria(args, ExternalProgram::Stderr_To_Stdout);
      int nLine = 0;

      //Process response
      for(std::string ariaResponse( aria.receiveLine());
          ariaResponse.length();
          ariaResponse = aria.receiveLine())
      {
        //cout << ariaResponse;

        if (!ariaResponse.substr(0,31).compare("Exception: Authorization failed") )
        {
            ZYPP_THROW(MediaUnauthorizedException(
                  _url, "Login failed.", "Login failed", "auth hint"
                ));
        }
        if (!ariaResponse.substr(0,29).compare("Exception: Resource not found") )
        {
            ZYPP_THROW(MediaFileNotFoundException(_url, filename));
        }

        if (!ariaResponse.substr(0,9).compare("[#2 SIZE:"))
        {
          if (!nLine)
          {
            size_t left_bound = ariaResponse.find('(',0) + 1;
            size_t count = ariaResponse.find('%',left_bound) - left_bound;
            //cout << ariaResponse.substr(left_bound, count) << endl;
            //progressData.toMax();
            report->progress ( std::atoi(ariaResponse.substr(left_bound, count).c_str()), _url, -1, -1 );
            nLine = 1;
          }
          else
          {
            nLine = 0;
          }
        }
      }

      aria.close();

      report->finish( _url ,  zypp::media::DownloadProgressReport::NO_ERROR, "");
      retry = false;
    }

    // retry with proper authentication data
    catch (MediaUnauthorizedException & ex_r)
    {
      if(authenticate(ex_r.hint(), !retry))
        retry = true;
      else
      {
        report->finish(fileurl, zypp::media::DownloadProgressReport::ACCESS_DENIED, ex_r.asUserHistory());
        ZYPP_RETHROW(ex_r);
      }

    }
    // unexpected exception
    catch (MediaException & excpt_r)
    {
      // FIXME: error number fix
      report->finish(fileurl, zypp::media::DownloadProgressReport::ERROR, excpt_r.asUserHistory());
      ZYPP_RETHROW(excpt_r);
    }
  }
  while (retry);

  report->finish(fileurl, zypp::media::DownloadProgressReport::NO_ERROR, "");
}

bool MediaAria2c::getDoesFileExist( const Pathname & filename ) const
{
    return MediaCurl::getDoesFileExist(filename);
}

bool MediaAria2c::doGetDoesFileExist( const Pathname & filename ) const
{
    return MediaCurl::doGetDoesFileExist(filename);
}
    
void MediaAria2c::getDir( const Pathname & dirname, bool recurse_r ) const
{
    MediaCurl::getDir(dirname, recurse_r);
}

bool MediaAria2c::authenticate(const std::string & availAuthTypes, bool firstTry) const
{
    return false;
}


void MediaAria2c::getDirInfo( std::list<std::string> & retlist,
                               const Pathname & dirname, bool dots ) const
{
  getDirectoryYast( retlist, dirname, dots );
}

void MediaAria2c::getDirInfo( filesystem::DirContent & retlist,
                            const Pathname & dirname, bool dots ) const
{
  getDirectoryYast( retlist, dirname, dots );
}

std::string MediaAria2c::getAria2cVersion()
{
    const char* argv[] =
    {
        _aria2cPath.c_str(),
      "--version",
      NULL
    };

    ExternalProgram aria(argv, ExternalProgram::Stderr_To_Stdout);

    std::string vResponse = aria.receiveLine();
    aria.close();
    return str::trim(vResponse);
}

#define ARIA_DEFAULT_BINARY "/usr/bin/aria2c"

Pathname MediaAria2c::whereisAria2c()
{
    Pathname aria2cPathr(ARIA_DEFAULT_BINARY);

    const char* argv[] =
    {
      "which",
      "aria2c",
      NULL
    };

    ExternalProgram aria(argv, ExternalProgram::Stderr_To_Stdout);

    std::string ariaResponse( aria.receiveLine());
    int code = aria.close();

    if( code == 0 )
    {
        aria2cPathr = str::trim(ariaResponse);
        MIL << "We will use aria2c located here:  " << aria2cPathr << endl;
    }
    else
    {
        MIL << "We don't know were is ari2ac binary. We will use aria2c located here:  " << aria2cPathr << endl;
    }

    return aria2cPathr;
}

} // namespace media
} // namespace zypp
//
