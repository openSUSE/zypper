/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/repo/RepoFileReader.cc
 *
*/
#include <iostream>
#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/base/Regex.h"
#include "zypp/base/InputStream.h"
#include "zypp/base/UserRequestException.h"

#include "zypp/parser/IniDict.h"
#include "zypp/parser/RepoFileReader.h"

using std::endl;
using zypp::parser::IniDict;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace parser
  { /////////////////////////////////////////////////////////////////

    /**
   * \short List of RepoInfo's from a file.
   * \param file pathname of the file to read.
   */
    static void repositories_in_stream( const InputStream &is,
                                        const RepoFileReader::ProcessRepo &callback,
                                        const ProgressData::ReceiverFnc &progress )
    {
      parser::IniDict dict(is);
      for ( parser::IniDict::section_const_iterator its = dict.sectionsBegin();
            its != dict.sectionsEnd();
            ++its )
      {
        RepoInfo info;
        info.setAlias(*its);
        Url url;

        for ( IniDict::entry_const_iterator it = dict.entriesBegin(*its);
              it != dict.entriesEnd(*its);
              ++it )
        {
          //MIL << (*it).first << endl;
          if (it->first == "name" )
            info.setName(it-> second);
          else if ( it->first == "enabled" )
            info.setEnabled( str::strToTrue( it->second ) );
          else if ( it->first == "priority" )
            info.setPriority( str::strtonum<unsigned>( it->second ) );
          else if ( it->first == "baseurl" && !it->second.empty())
            url = it->second;
          else if ( it->first == "path" )
            info.setPath( Pathname(it->second) );
          else if ( it->first == "type" )
            info.setType(repo::RepoType(it->second));
          else if ( it->first == "autorefresh" )
            info.setAutorefresh( str::strToTrue( it->second ) );
          else if ( it->first == "mirrorlist" && !it->second.empty())
            info.setMirrorListUrl(Url(it->second));
          else if ( it->first == "gpgkey" && !it->second.empty())
          {
            std::vector<std::string> keys;
            str::split( it->second, std::back_inserter(keys) );
            if ( ! keys.empty() )
              info.setGpgKeyUrl( Url(*keys.begin()) );
          }
          else if ( it->first == "gpgcheck" )
            info.setGpgCheck( str::strToTrue( it->second ) );
	  else if ( it->first == "keeppackages" )
	    info.setKeepPackages( str::strToTrue( it->second ) );
	  else if ( it->first == "service" )
	    info.setService( it->second );
          else if ( it->first == "proxy" )
          {
	    if (it->second != "_none_" )
            {
              str::regex ex("^(.*):([0-9]+)$");
              str::smatch what;
              if(str::regex_match(it->second, what, ex)){
               url.setQueryParam("proxy", what[1]);
               url.setQueryParam("proxyport", what[2]);
              }
            }
          } else
            ERR << "Unknown attribute in [" << *its << "]: " << it->first << "=" << it->second << " ignored" << endl;
        }
        if (url.isValid())
            info.addBaseUrl(url);
        info.setFilepath(is.path());
        MIL << info << endl;
        // add it to the list.
        callback(info);
        //if (!progress.tick())
        //  ZYPP_THROW(AbortRequestException());
      }
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : RepoFileReader
    //
    ///////////////////////////////////////////////////////////////////

    RepoFileReader::RepoFileReader( const Pathname & repo_file,
                                    const ProcessRepo & callback,
                                    const ProgressData::ReceiverFnc &progress )
      : _callback(callback)
    {
      repositories_in_stream(InputStream(repo_file), _callback, progress);
    }

    RepoFileReader::RepoFileReader( const InputStream &is,
                                    const ProcessRepo & callback,
                                    const ProgressData::ReceiverFnc &progress )
      : _callback(callback)
    {
      repositories_in_stream(is, _callback, progress);
    }

    RepoFileReader::~RepoFileReader()
    {}


    std::ostream & operator<<( std::ostream & str, const RepoFileReader & obj )
    {
      return str;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace parser
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
