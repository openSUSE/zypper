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

#include "zypp/Target.h"
#include "zypp/ZYppFactory.h"

#include "zypp/media/MediaAria2c.h"
#include "zypp/media/proxyinfo/ProxyInfos.h"
#include "zypp/media/ProxyInfo.h"
#include "zypp/media/MediaUserAuth.h"
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
      "whereis",
      "-b",
      "aria2c",
      NULL
    };

    ExternalProgram aria(argv, ExternalProgram::Stderr_To_Stdout);
	   
    std::string ariaResponse( aria.receiveLine());
    string::size_type pos = ariaResponse.find('/', 0 );
    if( pos != string::npos )
        return true;
    else
        return false;
}

static const char *const anonymousIdHeader()
{
  // we need to add the release and identifier to the
  // agent string.
  // The target could be not initialized, and then this information
  // is not available.
  Target_Ptr target = zypp::getZYpp()->getTarget();

  static const std::string _value(
      str::form(
          "X-Zypp-AnonymousId: %s",
          target ? target->anonymousUniqueId().c_str() : "" )
  );
  return _value.c_str();
}

static const char *const distributionFlavorHeader()
{
  // we need to add the release and identifier to the
  // agent string.
  // The target could be not initialized, and then this information
  // is not available.
  Target_Ptr target = zypp::getZYpp()->getTarget();

  static const std::string _value(
      str::trim( str::form(
          "X-ZYpp-DistributionFlavor: %s",
          target ? target->distributionFlavor().c_str() : "" ) )
  );
  return _value.c_str();
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
    : MediaHandler( url_r, attach_point_hint_r,
                    "/", // urlpath at attachpoint
                    true ) // does_download
{
  MIL << "MediaAria2c::MediaAria2c(" << url_r << ", " << attach_point_hint_r << ")" << endl;
      
  if( !attachPoint().empty())
  {
    PathInfo ainfo(attachPoint());
    Pathname apath(attachPoint() + "XXXXXX");
    char    *atemp = ::strdup( apath.asString().c_str());
    char    *atest = NULL;
    if( !ainfo.isDir() || !ainfo.userMayRWX() ||
         atemp == NULL || (atest=::mkdtemp(atemp)) == NULL)
    {
      WAR << "attach point " << ainfo.path()
          << " is not useable for " << url_r.getScheme() << endl;
      setAttachPoint("", true);
    }
    else if( atest != NULL)
      ::rmdir(atest);

    if( atemp != NULL)
      ::free(atemp);
  }

   //At this point, we initialize aria2c path
   _aria2cPath = Pathname( whereisAria2c().asString() );

   //Get aria2c version
   _aria2cVersion = getAria2cVersion();
}

void MediaAria2c::attachTo (bool next)
{
   // clear last arguments
   _args.clear();   

  if ( next )
    ZYPP_THROW(MediaNotSupportedException(_url));

  if ( !_url.isValid() )
    ZYPP_THROW(MediaBadUrlException(_url));

  if( !isUseableAttachPoint(attachPoint()))
  {
    std::string mountpoint = createAttachPoint().asString();

    if( mountpoint.empty())
      ZYPP_THROW( MediaBadAttachPointException(url()));

    setAttachPoint( mountpoint, true);
  }

  disconnectFrom(); 

  // Build the aria command.
  _args.push_back(_aria2cPath.asString());
  _args.push_back(str::form("--user-agent=%s", agentString()));
  _args.push_back("--summary-interval=1");
  _args.push_back("--follow-metalink=mem");
  _args.push_back( "--check-integrity=true");
  
   // add the anonymous id.
   _args.push_back(str::form("--header=%s", anonymousIdHeader() ));
   _args.push_back(str::form("--header=%s", distributionFlavorHeader() ));
  // TODO add debug option
   
  // Transfer timeout
  {
    _xfer_timeout = TRANSFER_TIMEOUT;

    std::string param(_url.getQueryParam("timeout"));
    if( !param.empty())
    {
      long num = str::strtonum<long>( param);
      if( num >= 0 && num <= TRANSFER_TIMEOUT_MAX)
        _xfer_timeout = num;
    }
  }

  _args.push_back( str::form("--connect-timeout=%d", CONNECT_TIMEOUT));

  // TODO limit redirections
  // TODO Implement certificate validation

  // FTP defaults to anonymous


  if ( _url.getUsername().empty() )
  {
    if ( _url.getScheme() == "ftp" )
    {
      string id = "yast2@";
      id += VERSION;
      DBG << "Anonymous FTP identification: '" << id << "'" << endl;
      _userpwd = "anonymous:" + id;
    }
  } 
  else 
  {
     if ( _url.getScheme() == "ftp" )
     { 
         _args.push_back(str::form("--ftp-user=%s", _url.getUsername().c_str() ));
     }
     else if ( _url.getScheme() == "http" ||
               _url.getScheme() == "https" )
    {
        _args.push_back(str::form("--http-user=%s", _url.getUsername().c_str() ));
    }
     
    if ( _url.getPassword().size() )
    {
      if ( _url.getScheme() == "ftp" )
      { 
          _args.push_back(str::form("--ftp-passwd=%s", _url.getPassword().c_str() ));
      }
      else if ( _url.getScheme() == "http" ||
               _url.getScheme() == "https" )
      {
          _args.push_back(str::form("--http-passwd=%s", _url.getPassword().c_str() ));
      }
    }
  }

  // note, aria2c does not support setting the auth type with
  // (basic, digest yet)
  

  /*---------------------------------------------------------------*
   CURLOPT_PROXY: host[:port]

   Url::option(proxy and proxyport)
   If not provided, /etc/sysconfig/proxy is evaluated
   *---------------------------------------------------------------*/

  _proxy = _url.getQueryParam( "proxy" );

  if ( ! _proxy.empty() )
  {
    string proxyport( _url.getQueryParam( "proxyport" ) );
    if ( ! proxyport.empty() ) {
      _proxy += ":" + proxyport;
    }
  }
  else
  {

    ProxyInfo proxy_info (ProxyInfo::ImplPtr(new ProxyInfoSysconfig("proxy")));

    if ( proxy_info.enabled())
    {
      bool useproxy = true;

      std::list<std::string> nope = proxy_info.noProxy();
      for (ProxyInfo::NoProxyIterator it = proxy_info.noProxyBegin();
           it != proxy_info.noProxyEnd();
           it++)
      {
        std::string host( str::toLower(_url.getHost()));
        std::string temp( str::toLower(*it));

        // no proxy if it points to a suffix
        // preceeded by a '.', that maches
        // the trailing portion of the host.
        if( temp.size() > 1 && temp.at(0) == '.')
        {
          if(host.size() > temp.size() &&
             host.compare(host.size() - temp.size(), temp.size(), temp) == 0)
          {
            DBG << "NO_PROXY: '" << *it  << "' matches host '"
                                 << host << "'" << endl;
            useproxy = false;
            break;
          }
        }
        else
        // no proxy if we have an exact match
        if( host == temp)
        {
          DBG << "NO_PROXY: '" << *it  << "' matches host '"
                               << host << "'" << endl;
          useproxy = false;
          break;
        }
      }

      if ( useproxy ) {
        _proxy = proxy_info.proxy(_url.getScheme());
      }
    }
  }

  DBG << "Proxy: " << (_proxy.empty() ? "-none-" : _proxy) << endl;

  if ( ! _proxy.empty() )
  {
      _args.push_back(str::form("--http-proxy=%s", _proxy.c_str() ));

     /*---------------------------------------------------------------*
     CURLOPT_PROXYUSERPWD: [user name]:[password]

     Url::option(proxyuser and proxypassword) -> CURLOPT_PROXYUSERPWD
     If not provided, $HOME/.curlrc is evaluated
     *---------------------------------------------------------------*/

    _proxyuserpwd = _url.getQueryParam( "proxyuser" );

    if ( ! _proxyuserpwd.empty() ) {
        _args.push_back(str::form("--http-proxy-user=%s", _proxyuserpwd.c_str() ));
      
      string proxypassword( _url.getQueryParam( "proxypassword" ) );
      if ( ! proxypassword.empty() ) {
          _args.push_back(str::form("--http-proxy-passwd=%s", proxypassword.c_str() ));
      }
    }
  }

  //_currentCookieFile = _cookieFile.asString();
  //_args.push_back(str::form("--load-cookies=%s", _currentCookieFile.c_str()));
  //NOTE cookie jar?

  // FIXME: need a derived class to propelly compare url's
  MediaSourceRef media( new MediaSource(_url.getScheme(), _url.asString()));
  setMediaSource(media);
	
}

bool
MediaAria2c::checkAttachPoint(const Pathname &apoint) const
{
  return MediaHandler::checkAttachPoint( apoint, true, true);
}

void MediaAria2c::disconnectFrom()
{
}

void MediaAria2c::releaseFrom( const std::string & ejectDev )
{
  disconnect();
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

  ExternalProgram::Arguments args = _args;
  args.push_back(str::form("--dir=%s", target.dirname().c_str()));
  args.push_back(fileurl.asString());
  
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

        if (!ariaResponse.substr(0,9).compare("[#2 SIZE:")) {
                
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
        report->finish(fileurl, zypp::media::DownloadProgressReport::ACCESS_DENIED, ex_r.asUserString());
        ZYPP_RETHROW(ex_r);
      }

    }
    // unexpected exception
    catch (MediaException & excpt_r)
    {
      // FIXME: error number fix
      report->finish(fileurl, zypp::media::DownloadProgressReport::ERROR, excpt_r.asUserString());
      ZYPP_RETHROW(excpt_r);
    }
  }
  while (retry);

  report->finish(fileurl, zypp::media::DownloadProgressReport::NO_ERROR, "");
}

bool MediaAria2c::getDoesFileExist( const Pathname & filename ) const
{
  bool retry = false;
  AuthData auth_data;

  do
  {
    try
    {
      return doGetDoesFileExist( filename );
    }
    // authentication problem, retry with proper authentication data
    catch (MediaUnauthorizedException & ex_r)
    {
      if(authenticate(ex_r.hint(), !retry))
        retry = true;
      else
        ZYPP_RETHROW(ex_r);
    }
    // unexpected exception
    catch (MediaException & excpt_r)
    {
      ZYPP_RETHROW(excpt_r);
    }
  }
  while (retry);

  return false;
}

bool MediaAria2c::doGetDoesFileExist( const Pathname & filename ) const
{
	
  DBG << filename.asString() << endl;
  return true;
}

void MediaAria2c::getDir( const Pathname & dirname, bool recurse_r ) const
{
  filesystem::DirContent content;
  getDirInfo( content, dirname, /*dots*/false );

  for ( filesystem::DirContent::const_iterator it = content.begin(); it != content.end(); ++it ) {
      Pathname filename = dirname + it->name;
      int res = 0;

      switch ( it->type ) {
      case filesystem::FT_NOT_AVAIL: // old directory.yast contains no typeinfo at all
      case filesystem::FT_FILE:
        getFile( filename );
        break;
      case filesystem::FT_DIR: // newer directory.yast contain at least directory info
        if ( recurse_r ) {
          getDir( filename, recurse_r );
        } else {
          res = assert_dir( localPath( filename ) );
          if ( res ) {
            WAR << "Ignore error (" << res <<  ") on creating local directory '" << localPath( filename ) << "'" << endl;
          }
        }
        break;
      default:
        // don't provide devices, sockets, etc.
        break;
      }
  }
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
      "whereis",
      "-b",
      "aria2c",
      NULL
    };

    ExternalProgram aria(argv, ExternalProgram::Stderr_To_Stdout);
	   
    std::string ariaResponse( aria.receiveLine());
    aria.close();
    
    string::size_type pos = ariaResponse.find('/', 0 );
    if( pos != string::npos ) 
    {
        aria2cPathr = ariaResponse;
        string::size_type pose = ariaResponse.find(' ', pos + 1 );
        aria2cPathr = ariaResponse.substr( pos , pose - pos );
        MIL << "We will use aria2c located here:  " << ariaResponse.substr( pos , pose - pos) << endl;
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
