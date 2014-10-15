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
#include "zypp/base/LogTools.h"
#include "zypp/base/String.h"
#include "zypp/base/Regex.h"
#include "zypp/base/InputStream.h"
#include "zypp/base/UserRequestException.h"

#include "zypp/parser/IniDict.h"
#include "zypp/parser/RepoFileReader.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  namespace parser
  {
    ///////////////////////////////////////////////////////////////////
    namespace {

      ///////////////////////////////////////////////////////////////////
      /// \class RepoFileParser
      /// \brief Modified \ref IniDict to allow parsing multiple 'baseurl=' entries
      ///////////////////////////////////////////////////////////////////
      class RepoFileParser : public IniDict
      {
      public:
	RepoFileParser( const InputStream & is_r )
	{ read( is_r ); }

	using IniDict::consume;	// don't hide overloads we don't redefine here

	virtual void consume( const std::string & section_r, const std::string & key_r, const std::string & value_r )
	{
	  if ( key_r == "baseurl" )
	  {
	    setInBaseurls( true );
	    _baseurls[section_r].push_back( Url(value_r) );
	  }
	  else
	  {
	    setInBaseurls( false );
	    IniDict::consume( section_r, key_r, value_r );
	  }
	}

	virtual void garbageLine( const std::string & section_r, const std::string & line_r )
	{
	  if ( _inBaseurls )
	    _baseurls[section_r].push_back( Url(line_r) );
	  else
	    IniDict::garbageLine( section_r, line_r );	// throw
	}

	std::list<Url> & baseurls( const std::string & section_r )
	{ return _baseurls[section_r]; }

      private:
	void setInBaseurls( bool yesno_r )
	{ if ( _inBaseurls != yesno_r ) _inBaseurls = yesno_r; }

	DefaultIntegral<bool,false> _inBaseurls;
	std::map<std::string,std::list<Url>> _baseurls;
      };

    } //namespace
    ///////////////////////////////////////////////////////////////////

    /**
   * \short List of RepoInfo's from a file.
   * \param file pathname of the file to read.
   */
    static void repositories_in_stream( const InputStream &is,
                                        const RepoFileReader::ProcessRepo &callback,
                                        const ProgressData::ReceiverFnc &progress )
    {
      RepoFileParser dict(is);
      for_( its, dict.sectionsBegin(), dict.sectionsEnd() )
      {
        RepoInfo info;
        info.setAlias(*its);
	std::string proxy;
	std::string proxyport;

        for_( it, dict.entriesBegin(*its), dict.entriesEnd(*its) )
        {
          //MIL << (*it).first << endl;
          if (it->first == "name" )
            info.setName(it-> second);
          else if ( it->first == "enabled" )
            info.setEnabled( str::strToTrue( it->second ) );
          else if ( it->first == "priority" )
            info.setPriority( str::strtonum<unsigned>( it->second ) );
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
	    // Translate it into baseurl queryparams
	    // NOTE: The hack here does not add proxy to mirrorlist urls but the
	    //       original code worked without complains, so keep it for now.
	    static const str::regex ex( ":[0-9]+$" );	// portspec
	    str::smatch what;
	    if ( str::regex_match( it->second, what, ex ) )
	    {
	      proxy = it->second.substr( 0, it->second.size() - what[0].size() );
	      proxyport = what[0].substr( 1 );
	    }
	    else
	    {
	      proxy = it->second;
	    }
	  }
          else
            ERR << "Unknown attribute in [" << *its << "]: " << it->first << "=" << it->second << " ignored" << endl;
        }

	USR << dict.baseurls( *its ) << endl;
	for ( auto & url : dict.baseurls( *its ) )
	{
	  if ( ! proxy.empty() && url.getQueryParam( "proxy" ).empty() )
	  {
	    url.setQueryParam( "proxy", proxy );
	    url.setQueryParam( "proxyport", proxyport );
	  }
	  info.addBaseUrl( url );
	}

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

  } // namespace parser
  ///////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
