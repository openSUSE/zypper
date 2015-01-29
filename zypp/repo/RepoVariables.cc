/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include "zypp/base/LogTools.h"
#include "zypp/base/String.h"
#include "zypp/base/Regex.h"

#include "zypp/ZConfig.h"
#include "zypp/Target.h"
#include "zypp/Arch.h"
#include "zypp/repo/RepoVariables.h"
#include "zypp/base/NonCopyable.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{
  namespace env
  {
    /** Use faked releasever (e.g. for 'zupper dup' to next distro version */
    inline std::string ZYPP_REPO_RELEASEVER()
    {
      const char * env = getenv("ZYPP_REPO_RELEASEVER");
      return( env ? env : "" );
    }
  }

  ///////////////////////////////////////////////////////////////////
  namespace repo
  {
    ///////////////////////////////////////////////////////////////////
    namespace
    {
      /** \brief Provide lazy initialized repo variables
       */
      struct ReplacerData : private zypp::base::NonCopyable
      {
	typedef const std::string & (ReplacerData::*Getter)() const;

	const std::string & arch() const
	{
	  assertArchStr();
	  return _arch;
	}

	const std::string & basearch() const
	{
	  assertArchStr();
	  return _basearch;
	}

	const std::string & releasever() const
	{
	  assertReleaseverStr();
	  return _releasever;
	}

	const std::string & releaseverMajor() const
	{
	  assertReleaseverStr();
	  return _releaseverMajor;
	}

	const std::string & releaseverMinor() const
	{
	  assertReleaseverStr();
	  return _releaseverMinor;
	}

      private:
	void assertArchStr() const
	{
	  if ( _arch.empty() )
	  {
	    Arch arch( ZConfig::instance().systemArchitecture() );
	    _arch = arch.asString();
	    _basearch = arch.baseArch().asString();
	  }
	}
	void assertReleaseverStr() const
	{
	  if ( _releasever.empty() )
	  {
	    _releasever = env::ZYPP_REPO_RELEASEVER();
	    if( _releasever.empty() )
	      _releasever = Target::distributionVersion( Pathname()/*guess*/ );
	    else
	      WAR << "ENV overwrites $releasever=" << _releasever << endl;

	    // split major/minor for SLE
	    std::string::size_type pos = _releasever.find( "." );
	    if ( pos == std::string::npos )
	    {
	      _releaseverMajor = _releasever;
	      _releaseverMinor.clear();
	    }
	    else
	    {
	      _releaseverMajor = _releasever.substr( 0, pos );
	      _releaseverMinor = _releasever.substr( pos+1 ) ;
	    }
	  }
	}
      private:
	mutable std::string _arch;
	mutable std::string _basearch;
	mutable std::string _releasever;
	mutable std::string _releaseverMajor;
	mutable std::string _releaseverMinor;
      };

     /** \brief Replace repo variables on demand
       *
       * Initialisation of repo variables is delayed until they actually occur in
       * a string.
       */
      std::string replacer( std::string value_r )
      {
	std::string ret;
	if ( ! value_r.empty() )
	{
	  static const str::regex rxVAR( "^([^$]*)\\$(\\{[[:alnum:]_]+\\}|[[:alnum:]_]+)([^$]*)" );
	  str::smatch what;
	  while ( str::regex_match( value_r, what, rxVAR ) )
	  {
	    ReplacerData::Getter getter = nullptr;

	    const char * varStart = value_r.c_str() + what.begin( 2 );
	    std::string::size_type varSize = what.size( 2 );
	    if ( *varStart == '{' )	// enclosed in {}
	    {
	      ++varStart;
	      varSize -= 2;
	    }

	    switch ( varSize )
	    {
#define ASSIGN_IF(NAME,GETTER) if ( ::strncmp( varStart, NAME, varSize ) == 0 ) getter = GETTER

	      case  4:	ASSIGN_IF( "arch",		&ReplacerData::arch );			break;
	      case  8:	ASSIGN_IF( "basearch",		&ReplacerData::basearch );		break;
	      case 10:	ASSIGN_IF( "releasever",	&ReplacerData::releasever );		break;
	      case 16:	ASSIGN_IF( "releasever_major",	&ReplacerData::releaseverMajor );
	           else	ASSIGN_IF( "releasever_minor",	&ReplacerData::releaseverMinor );	break;
#undef ASSIGN_IF
	    }

	    if ( getter )	// known var?
	    {
	      static const ReplacerData _data;
	      if ( what.size( 1 ) > 0 ) ret += what[1];	// pre
	      ret += (_data.*getter)();			// var
	      if ( what.size( 3 ) > 0 ) ret += what[3];	// post
	    }
	    else
	    {
	      ret += what[0];	// unchanged
	    }

	    value_r.erase( 0, what.size( 0 ) );
	    if ( value_r.empty() )
	      break;
	  }
	  if ( ! value_r.empty() )
	    ret += std::move(value_r);		// no match
	}
	return ret;
      }
    } // namespace
    ///////////////////////////////////////////////////////////////////

    std::string RepoVariablesStringReplacer::operator()( const std::string & value ) const
    {
      return replacer( value );
    }

    Url RepoVariablesUrlReplacer::operator()( const Url & value ) const
    {
      Url newurl( value );
      newurl.setPathData( replacer( value.getPathData() ) );
      newurl.setQueryString( replacer( value.getQueryString() ) );
      return newurl;
    }

  } // namespace repo
  ///////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
