#include <iostream>
#include <fstream>

#include "zypp/base/Logger.h"
#include "zypp/base/IOStream.h"
#include "zypp/Pathname.h"
#include "zypp/PathInfo.h"

#include "zypp/media/CurlConfig.h"

using namespace std;

namespace zypp
{
  namespace media
  {


  ///////////////////////////////////////////////////////////////////
  //
  //  METHOD NAME : CurlConfig::parseConfig
  //  METHOD TYPE : int
  //
  int CurlConfig::parseConfig(CurlConfig & config, const std::string & filename)
  {
    Pathname curlrcFile;

    if(filename.empty())
    {
      // attempts to load .curlrc from the homedir
      char *home = getenv("HOME");
      if(home)
        curlrcFile = string( home ) + string( "/.curlrc" );
    }
    else
      curlrcFile = filename;

    PathInfo h_info(curlrcFile.dirname(), PathInfo::LSTAT);
    PathInfo c_info(curlrcFile,           PathInfo::LSTAT);

    if( h_info.isDir()  && h_info.userMayRX() &&
        c_info.isFile() && c_info.userMayR() )
    {
      MIL << "Going to parse " << curlrcFile << endl;
    }
    else
    {
      char buf[32] = "";
      WAR << "Not allowed to parse '" << curlrcFile
          << "': dir/file owner: " << h_info.owner() << "/" << c_info.owner()
          << ", process uid: " << getuid()
          << " (" << (!getlogin_r(buf, 31) ? buf : "") << ")" << std::endl;

      return 1;
    }

    ifstream inp(curlrcFile.c_str());
    for(iostr::EachLine in( inp ); in; in.next())
    {
      string line = str::trim(*in);

      // skip empty lines and comments
      if (line.empty())
        continue;
      switch (line[0])
      {
      case '#':
      case '/':
      case '\r':
      case '\n':
      case '*':
      case '\0':
        continue;
      }

      // DBG << "line " << in.lineNo() << ": " << line << endl; // can't log passwords

      const char * beg = line.c_str();
      const char * cur = beg;

// space, '=' and ':' are all valid separators in curlrc
#define ISSEP(x) (((x)=='=') || ((x) == ':') || isspace(x))

      // skip leading dashes (they are optional)
      while (*cur && *cur == '-')
        cur++;
      beg = cur;

      // skip non-separator characters
      while (*cur && !ISSEP(*cur))
        cur++;

      string option(beg, cur - beg);

      // skip separator characters
      while (*cur && ISSEP(*cur))
        cur++;

      // rewind to the end of the line
      beg = cur;
      while (*cur)
        cur++;

      string value(beg, cur - beg);

      DBG << "GOT: " << option << endl;

      if (!value.empty())
      {
        // quoted parameter
        if (value[0] == '\"')
        {
          // remove the quotes
          string::size_type pos = value.rfind('\"');
          bool cut_last =
            pos == value.size() - 1 && pos > 1 && value[pos-1] != '\\';
          value = value.substr(1,
              cut_last ? value.size() - 2 : value.size() - 1);

          // replace special characters:
          pos = 0;
          while ((pos = value.find('\\', pos)) != string::npos)
          {
            // just erase the backslash if it is found at the end
            if (pos == value.size() - 1)
            {
              value = value.erase(pos, 1);
              break;
            }

            switch(value[pos+1])
            {
            case 't':
              value = value.replace(pos, 2, "\t");
              break;
            case 'n':
              value = value.replace(pos, 2, "\n");
              break;
            case 'r':
              value = value.replace(pos, 2, "\r");
              break;
            case 'v':
              value = value.replace(pos, 2, "\v");
              break;
            case '\\':
              value = value.erase(pos++, 1);
              break;
            default:;
              value = value.erase(pos, 1);
            }
          }
        }

        // DBG << "PARAM: " << value << endl; // can't log passwords
      }

      CurlConfig::setParameter(config, option, value);
    } // for EachLine in curlrc

    return 0;
  }


  ///////////////////////////////////////////////////////////////////
  //
  //  METHOD NAME : CurlConfig::setParameter
  //  METHOD TYPE : int
  //
  int CurlConfig::setParameter(CurlConfig & config,
                               const std::string & option,
                               const std::string & value)
  {
    if (option == "proxy-user")
      config.proxyuserpwd = value;
    // add more curl config data here as they become needed
    // else if (option == "foo")
    else
      DBG << "Ignoring option " << option << endl;

    return 0;
  }


  } // namespace media
} // namespace zypp
