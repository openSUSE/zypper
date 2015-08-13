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
#include "zypp/base/Regex.h"
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
	std::map<std::string,std::pair<std::string,ServiceInfo::RepoState>> repoStates;	// <repo_NUM,< alias,RepoState >>

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
	  else if ( it->first == "ttl_sec" )
	    service.setTtl( str::strtonum<Date::Duration>(it->second) );
	  else if ( it->first == "lrf_dat" )
	    service.setLrf( Date( it->second ) );
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
          else if ( str::startsWith( it->first, "repo_" ) )
	  {
	    static str::regex rxexpr( "([0-9]+)(_(.*))?" );
	    str::smatch what;
	    if ( str::regex_match( it->first.c_str()+5/*repo_*/, what, rxexpr ) )
	    {
	      std::string tag( what[1] );
	      if (  what.size() > 3 )
	      {
		// attribute
		if ( what[3] == "enabled" )
		  repoStates[tag].second.enabled = str::strToBool( it->second, repoStates[tag].second.enabled );
		else if ( what[3] == "autorefresh" )
		  repoStates[tag].second.autorefresh = str::strToBool( it->second, repoStates[tag].second.autorefresh );
		else if ( what[3] == "priority" )
		  str::strtonum( it->second, repoStates[tag].second.priority );
		else
		  ERR << "Unknown attribute " << it->first << " ignored" << endl;
	      }
	      else
	      {
		// alias
		repoStates[tag].first = it->second;
	      }
	    }
	    else
	      ERR << "Unknown attribute " << it->first << " ignored" << endl;
	  }
          else
            ERR << "Unknown attribute " << it->first << " ignored" << endl;
        }

	if ( ! repoStates.empty() )
	{
	  ServiceInfo::RepoStates data;
	  for ( const auto & el : repoStates )
	  {
	    if ( el.second.first.empty() )
	      ERR << "Missing alias for repo_" << el.first << "; ignore entry" << endl;
	    else
	      data[el.second.first] = el.second.second;
	  }
	  if ( ! data.empty() )
	    service.setRepoStates( std::move(data) );
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
