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
#include <fstream>
#include <boost/lexical_cast.hpp>

#include "zypp/base/Logger.h"
#include "zypp/base/Regex.h"
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

#include "zypp/TmpPath.h"

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
                      filesystem::TmpPath &credentials,
                      const Url &url,
                      const Pathname &destination,
                      ExternalProgram::Arguments &args )
{

    // options that are not passed in the command line
    // like credentials, every string is in the
    // opt=val format
    list<string> file_options;

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
        MIL << "Passing " << url.getScheme() << " credentials '" << s.username() << ':' << (s.password().empty() ? "" : "PASSWORD")<< "'" << endl;
        if ( url.getScheme() == "ftp" )
            file_options.push_back(str::form("ftp-user=%s", s.username().c_str() ));
        else if ( url.getScheme() == "http" ||
                  url.getScheme() == "https" )
            file_options.push_back(str::form("http-user=%s", s.username().c_str() ));

        if ( s.password().size() )
        {
            if ( url.getScheme() == "ftp" )
                file_options.push_back(str::form("ftp-passwd=%s", s.password().c_str() ));
            else if ( url.getScheme() == "http" ||
                      url.getScheme() == "https" )
                file_options.push_back(str::form("http-passwd=%s", s.password().c_str() ));
        }
    }

    if ( s.proxyEnabled() )
    {
        args.push_back(str::form("--http-proxy=%s", s.proxy().c_str() ));
        if ( ! s.proxyUsername().empty() )
        {
            MIL << "Passing " << /*url.getScheme()*/"http" << "-proxy credentials '" << s.proxyUsername() << ':' << (s.proxyPassword().empty() ? "" : "PASSWORD")<< "'" << endl;
            file_options.push_back(str::form("http-proxy-user=%s", s.proxyUsername().c_str() ));
            if ( ! s.proxyPassword().empty() )
                file_options.push_back(str::form("http-proxy-passwd=%s", s.proxyPassword().c_str() ));
        }
    }

    if ( ! destination.empty() )
        args.push_back(str::form("--dir=%s", destination.c_str()));

    // now append the file if there are hidden options
    if ( ! file_options.empty() )
    {
        filesystem::TmpFile tmp;
        ofstream outs( tmp.path().c_str() );
        for_( it, file_options.begin(), file_options.end() )
            outs << *it << endl;
        outs.close();

        credentials = tmp;
        args.push_back(str::form("--conf-path=%s", credentials.path().c_str()));
    }

    // Credentials are passed via --{ftp,http}-{user,passwd}.
    // Aria does not like them being repeated in the url. (bnc #544634)
    args.push_back(url.asString( url.getViewOptions()
                               - url::ViewOptions::WITH_USERNAME
                               - url::ViewOptions::WITH_PASSWORD ).c_str());
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
       , Target::targetDistribution( Pathname()/*guess root*/ ).c_str()
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

void MediaAria2c::getFile( const Pathname & filename ) const
{
    // Use absolute file name to prevent access of files outside of the
    // hierarchy below the attach point.
    getFileCopy(filename, localPath(filename).absolutename());
}

void MediaAria2c::getFileCopy( const Pathname & filename , const Pathname & target) const
{
  callback::SendReport<DownloadProgressReport> report;

  Url fileurl(getFileUrl(filename));

  bool retry = false;

  ExternalProgram::Arguments args;

  filesystem::TmpPath credentials;
  fillAriaCmdLine(_aria2cVersion, _settings, credentials, fileurl, target.dirname(), args);

  do
  {
    try
    {
      report->start(fileurl, target.asString() );

      ExternalProgram aria(args, ExternalProgram::Stderr_To_Stdout);

      // extended regex for parsing of progress lines
      // progress line like: [#1 SIZE:8.3MiB/10.1MiB(82%) CN:5 SPD:6899.88KiB/s]
      // but since 1.4.0:    [#1 SIZE:8.3MiB/10.1MiB(82%) CN:5 SPD:899.8KiBs]
      //       (bnc #513944) [#1 SIZE:8.3MiB/10.1MiB(82%) CN:5 SPD:3.8MiBs]
      //                     [#1 SIZE:8.3MiB/10.1MiB(82%) CN:5 SPD:38Bs]
      // later got also ETA: [#1 SIZE:8.3MiB/10.1MiB(82%) CN:5 SPD:38Bs ETA:02s]
      static str::regex rxProgress(
          "^\\[#[0-9]+ SIZE:[0-9\\.]+(|Ki|Mi|Ti)B/[0-9\\.]+(|Ki|Mi|Ti)B\\(?([0-9]+)?%?\\)? CN:[0-9]+ SPD:([0-9\\.]+)(|Ki|Mi|Ti)Bs.*\\]$");

      // whether we received correct progress line before corresponding FILE line
      bool gotProgress = false;
      // download progress in %
      int progress = 0;
      // current download speed in bytes
      double current_speed = 0;
      // download speed in bytes
      double average_speed = 0;
      // number of speed measurements
      long average_speed_count = 0;

      // here we capture aria output exceptions
      vector<string> ariaExceptions;

      // whether it makes sense to retry with --continue
      bool partialDownload = false;
      // whether user request abort of the download
      bool userAbort = false;

      //Process response
      for(std::string ariaResponse( aria.receiveLine());
          ariaResponse.length();
          ariaResponse = aria.receiveLine())
      {
        string line = str::trim(ariaResponse);
        // INT << line << endl;

        // look for the progress line and save parsed values until we find
        // a string with FILE: later.
        if ( str::hasPrefix(line, "[#") )
        {
          str::smatch progressValues;
          if (( gotProgress = str::regex_match(line, progressValues, rxProgress) ))
          {
            // INT << "got: progress: '" << progressValues[3]
            //     << "' speed: '" << progressValues[4] << " "
            //     << progressValues[5] << "Bs'" << endl;

            // get the percentage (progress) data
            progress = std::atoi(progressValues[3].c_str());

            // get the speed

            int factor = 1; // B/KiB/MiB multiplication factor
            if (progressValues[5] == "Ki")
              factor = 1024;
            else if (progressValues[5] == "Mi")
              factor = 0x100000;
            else if (progressValues[5] == "Ti")
              factor = 0x40000000;

            try {
              current_speed = boost::lexical_cast<double>(progressValues[4]);
              // convert to and work with bytes everywhere (bnc #537870)
              current_speed *= factor;
            }
            catch (const std::exception&) {
              ERR << "Can't parse speed from '" << progressValues[4] << "'" << endl;
              current_speed = 0;
            }
          }
          else
            ERR << "Can't parse progress line '" << line << "'" << endl;
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
        // The file line tells which file corresponds to the previous progress,
        // eg.: FILE: ./packages.FL.gz
        else if ( str::hasPrefix(line, "FILE: ") )
        {
          // get the FILE name
          string theFile(line.substr(6, line.size()));
          // is the report about the filename we are downloading?
          // aria may report progress about metalinks, torrent and
          // other stuff which is not the main transfer
          // the reported file is the url before the server emits a response
          // and then is reported as the target file
          if ( Pathname(theFile) == target || theFile == fileurl.asCompleteString() )
          {
            // once we find the FILE: line, progress has to be
            // non empty
            if ( gotProgress )
            {
              // we have a new average speed
              average_speed_count++;

              // this is basically A: average
              // ((n-1)A(n-1) + Xn)/n = A(n)
              average_speed =
                (((average_speed_count - 1)*average_speed) + current_speed)
                / average_speed_count;

              if (!partialDownload && progress > 0)
                partialDownload = true;

              if ( ! report->progress ( progress, fileurl, average_speed, current_speed ) )
                userAbort = true;

              // clear the flag to detect mismatches between [# and FILE: lines
              gotProgress = false;
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

      int code;
      if (userAbort)
      {
        aria.kill();
        code = 7;
      }
      else
        code = aria.close();

      switch ( code )
      {
        case 0: // success?
          if ( ! PathInfo( target ).isExist() )
          {
            // bnc #564816: aria2 might return 0 if an error occurred
            // _before_ the download actually started.

            // TranslatorExplanation: Failed to download <FILENAME> from <SERVERURL>.
            std::string msg( str::form(_("Failed to download %s from %s"),
                             filename.c_str(), _url.asString().c_str() ) );

            MediaException e( msg );
            for_( it, ariaExceptions.begin(), ariaExceptions.end() )
              e.addHistory( *it );

            ZYPP_THROW( e );
          }
          break;

        case 2: // timeout
        {
          MediaTimeoutException e(_url);
          for_(it, ariaExceptions.begin(), ariaExceptions.end())
              e.addHistory(*it);
          ZYPP_THROW(e);
        }
        break;

        case 3: // not found
        case 4: // max notfound reached
        {
          MediaFileNotFoundException e(_url, filename);
          for_(it, ariaExceptions.begin(), ariaExceptions.end())
              e.addHistory(*it);
          ZYPP_THROW(e);
        }
        break;

        case 5: // too slow
        case 6: // network problem
        case 7: // unfinished downloads (ctr-c)
        case 1: // unknown
        default:
        {
          if ( partialDownload )
          {
            // Ask for retry on partial downloads, when it makes sense to retry with --continue!
            // Other errors are handled by the layers above.
            MediaException e(str::form(_("Download interrupted at %d%%"), progress ));
            for_(it, ariaExceptions.begin(), ariaExceptions.end())
              e.addHistory(*it);

            DownloadProgressReport::Action action = report->problem( _url, DownloadProgressReport::ERROR, e.asUserHistory() );
            if ( action == DownloadProgressReport::RETRY )
            {
              retry = true;
              continue;
            }
          }

          string msg;
          if (userAbort)
            msg = _("Download interrupted by user");
          else
            // TranslatorExplanation: Failed to download <FILENAME> from <SERVERURL>.
            msg = str::form(_("Failed to download %s from %s"),
                filename.c_str(), _url.asString().c_str());

          MediaException e(msg);
          for_(it, ariaExceptions.begin(), ariaExceptions.end())
              e.addHistory(*it);

          ZYPP_THROW(e);
        }
        break;
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
