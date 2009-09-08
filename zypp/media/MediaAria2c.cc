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
#include <vector>

#include <boost/lexical_cast.hpp>

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
#include "zypp/ZConfig.h"

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

#define  ARIA_BINARY "aria2c"

using namespace std;
using namespace zypp::base;

namespace zypp
{
namespace media
{

Pathname MediaAria2c::_cookieFile = "/var/lib/YaST2/cookies";
std::string MediaAria2c::_aria2cVersion = "WE DON'T KNOW ARIA2C VERSION";

//check if aria2c is present in the system
bool
MediaAria2c::existsAria2cmd()
{
  static const char* argv[] =
  {
    ARIA_BINARY,
    "--version",
    NULL
  };
  ExternalProgram aria( argv, ExternalProgram::Stderr_To_Stdout );
  return( aria.close() == 0 );
}

/**
 * comannd line for aria.
 * The argument list gets passed as reference
 * and it is filled.
 */
void fillAriaCmdLine( const string &ariaver,
                      const TransferSettings &s,
                      const Url &url,
                      const Pathname &destination,
                      ExternalProgram::Arguments &args )
{
    args.push_back(ARIA_BINARY);
    args.push_back(str::form("--user-agent=%s", s.userAgentString().c_str()));
    args.push_back("--summary-interval=1");
    args.push_back("--follow-metalink=mem");
    args.push_back("--check-integrity=true");
    args.push_back("--file-allocation=none");

    // save the stats of the mirrors and use them as input later
    Pathname statsFile = ZConfig::instance().repoCachePath() / "aria2.stats";
    args.push_back(str::form("--server-stat-of=%s", statsFile.c_str()));
    args.push_back(str::form("--server-stat-if=%s", statsFile.c_str()));
    args.push_back("--uri-selector=adaptive");

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

  fillAriaCmdLine(_aria2cVersion, _settings, fileurl, target.dirname(), args);

  do
  {
    try
    {
      report->start(fileurl, target.asString() );

      ExternalProgram aria(args, ExternalProgram::Stderr_To_Stdout);

      // progress line like: [#1 SIZE:8.3MiB/10.1MiB(82%) CN:5 SPD:6899.88KiB/s]
      // but since 1.4.0:    [#1 SIZE:8.3MiB/10.1MiB(82%) CN:5 SPD:899.8KiBs]
      //       (bnc #513944) [#1 SIZE:8.3MiB/10.1MiB(82%) CN:5 SPD:3.8MiBs]
      // we save it until we find a string with FILE: later
      string progressLine;
      // file line, which tell which file is the previous progress
      // ie: FILE: ./packages.FL.gz
      double average_speed = 0;
      long average_speed_count = 0;

      // here we capture aria output exceptions
      vector<string> ariaExceptions;

      //Process response
      for(std::string ariaResponse( aria.receiveLine());
          ariaResponse.length();
          ariaResponse = aria.receiveLine())
      {
        //cout << ariaResponse;
        string line = str::trim(ariaResponse);

        // look for the progress line and save it for later
        if ( str::hasPrefix(line, "[#") )
        {
          progressLine = line;
        }
        // save error messages for later
        else if ( str::hasPrefix(line, "Exception: ") )
        {
          // for auth exception, we throw
          if (!line.substr(0,31).compare("Exception: Authorization failed") )
          {
            ZYPP_THROW(MediaUnauthorizedException(
                       _url, "Login failed.", "Login failed", "auth hint"
            ));
          }
          // otherwise, remember the error
          string excpMsg = line.substr(10, line.size());
          DBG << "aria2c reported: '" << excpMsg << "'" << endl;
          ariaExceptions.push_back(excpMsg);
        }
        else if ( str::hasPrefix(line, "FILE: ") )
        {
          // get the FILE name
          Pathname theFile(line.substr(6, line.size()));
          // is the report about the filename we are downloading?
          // aria may report progress about metalinks, torrent and
          // other stuff which is not the main transfer
          if ( theFile == target )
          {
            // once we find the FILE: line, progress has to be
            // non empty
            if ( ! progressLine.empty() )
            {
              // get the percentage (progress) data
              int progress = 0;
              size_t left_bound = progressLine.find('(',0) + 1;
              size_t count = progressLine.find('%',left_bound) - left_bound;
              string progressStr = progressLine.substr(left_bound, count);

              if ( count != string::npos )
                progress = std::atoi(progressStr.c_str());
              else
                  ERR << "Can't parse progress from '" << progressStr << "'" << endl;

              // get the speed
              double current_speed = 0;
              left_bound = progressLine.find("SPD:",0) + 4;
              count = progressLine.find("KiB",left_bound);
              bool kibs = true; // KiBs ? (MiBs if false)
              if ( count == string::npos ) // try MiBs
              {
                count = progressLine.find("MiBs",left_bound);
                kibs = false;
              }
              if ( count != string::npos )
              { // convert the string to a double
                count -= left_bound;
                string speedStr = progressLine.substr(left_bound, count);
                try {
                  current_speed = boost::lexical_cast<double>(speedStr);
                }
                catch (const std::exception&) {
                  ERR << "Can't parse speed from '" << speedStr << "'" << endl;
                  current_speed = 0;
                }
              }

              // we have a new average speed
              average_speed_count++;

              // this is basically A: average
              // ((n-1)A(n-1) + Xn)/n = A(n)
              average_speed =
                (((average_speed_count - 1 )*average_speed) + current_speed)
                / average_speed_count;

              // note that aria report speed in kBps or MBps, while the report takes Bps
              report->progress ( progress, fileurl,
                  average_speed * (kibs ? 0x400 : 0x10000),
                  current_speed * (kibs ? 0x400 : 0x10000));
              // clear the progress line to detect mismatches between
              // [# and FILE: lines
              progressLine.clear();
            }
            else
            {
              WAR << "aria2c reported a file, but no progress data available" << endl;
            }

          }
          else
          {
            DBG << "Progress is not about '" << target << "' but '" << theFile << "'" << endl;
          }
        }
        else
        {
            // other line type, just ignore it.
        }
      }

      int code = aria.close();

      switch ( code )
      {
        // success
        case 0: // success
            break;
        case 2: // timeout
        {
          MediaTimeoutException e(_url);
          for_(it, ariaExceptions.begin(), ariaExceptions.end())
              e.addHistory(*it);
          ZYPP_THROW(e);
        }
        case 3: // not found
        case 4: // max notfound reached
        {
          MediaFileNotFoundException e(_url, filename);
          for_(it, ariaExceptions.begin(), ariaExceptions.end())
              e.addHistory(*it);
          ZYPP_THROW(e);
        }
        case 5: // too slow
        case 6: // network problem
        case 7: // unfinished downloads (ctr-c)
        case 1: // unknown
        default:
        {
          // TranslatorExplanation: Failed to download <FILENAME> from <SERVERURL>.
          MediaException e(str::form(_("Failed to download %s from %s"), filename.c_str(), _url.asString().c_str()));
          for_(it, ariaExceptions.begin(), ariaExceptions.end())
              e.addHistory(*it);

          ZYPP_THROW(e);
        }
      }

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
    static const char* argv[] =
    {
      ARIA_BINARY,
      "--version",
      NULL
    };
    ExternalProgram aria(argv, ExternalProgram::Stderr_To_Stdout);
    std::string vResponse( str::trim( aria.receiveLine() ) );
    aria.close();
    return vResponse;
}
} // namespace media
} // namespace zypp
//
