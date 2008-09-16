/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/RepoFileReader.cc
 *
*/
#include <iostream>
#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/base/InputStream.h"
#include "zypp/base/UserRequestException.h"

#include "zypp/parser/IniDict.h"
#include "zypp/parser/ServiceFileReader.h"
#include "zypp/ServiceInfo.h"

using std::endl;
using zypp::parser::IniDict;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace parser
  { /////////////////////////////////////////////////////////////////

    class ServiceFileReader::Impl
    {
    public:
      static void parseServices( const Pathname & file,
          const ServiceFileReader::ProcessService & callback );
    };

    void ServiceFileReader::Impl::parseServices( const Pathname & file,
                                  const ServiceFileReader::ProcessService & callback/*,
                                  const ProgressData::ReceiverFnc &progress*/ )
    {
      InputStream is(file);
      if( is.stream().fail() )
      {
        ZYPP_THROW(Exception("Failed to open service file"));
      }

      parser::IniDict dict(is);
      for ( parser::IniDict::section_const_iterator its = dict.sectionsBegin();
            its != dict.sectionsEnd();
            ++its )
      {
        MIL << (*its) << endl;

        ServiceInfo service(*its);

        for ( IniDict::entry_const_iterator it = dict.entriesBegin(*its);
              it != dict.entriesEnd(*its);
              ++it )
        {
          // MIL << (*it).first << endl;
          if ( it->first == "name" )
            service.setName( it->second );
          else if ( it->first == "url" && ! it->second.empty() )
            service.setUrl( Url (it->second) );
          else if ( it->first == "enabled" )
            service.setEnabled( str::strToTrue( it->second ) );
          else if ( it->first == "autorefresh" )
            service.setAutorefresh( str::strToTrue( it->second ) );
          else if ( it->first == "type" )
            service.setType( repo::ServiceType(it->second) );
          else if ( it->first == "repostoenable" )
          {
            std::vector<std::string> aliases;
            str::splitEscaped( it->second, std::back_inserter(aliases) );
            for_( ait, aliases.begin(), aliases.end() )
            {
              service.addRepoToEnable( *ait );
            }
          }
          else if ( it->first == "repostodisable" )
          {
            std::vector<std::string> aliases;
            str::splitEscaped( it->second, std::back_inserter(aliases) );
            for_( ait, aliases.begin(), aliases.end() )
            {
              service.addRepoToDisable( *ait );
            }
          }
          else
            ERR << "Unknown attribute " << it->first << " ignored" << endl;
        }

        MIL << "Linking ServiceInfo with file " << file << endl;
        service.setFilepath(file);

        // add it to the list.
        if ( !callback(service) )
          ZYPP_THROW(AbortRequestException());
      }
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : RepoFileReader
    //
    ///////////////////////////////////////////////////////////////////

    ServiceFileReader::ServiceFileReader( const Pathname & repo_file,
                                    const ProcessService & callback/*,
                                    const ProgressData::ReceiverFnc &progress */)
    {
      Impl::parseServices(repo_file, callback/*, progress*/);
      //MIL << "Done" << endl;
    }

    ServiceFileReader::~ServiceFileReader()
    {}

    std::ostream & operator<<( std::ostream & str, const ServiceFileReader & obj )
    {
      return str;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace parser
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
