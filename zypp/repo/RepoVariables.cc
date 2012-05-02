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

#include "zypp/ZConfig.h"
#include "zypp/Target.h"
#include "zypp/Arch.h"
#include "zypp/repo/RepoVariables.h"
#include "zypp/base/NonCopyable.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{
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
	const std::string & sysarch() const
	{
	  if ( _sysarch.empty() )
	    initArchStr();
	  return _sysarch;
	}

	const std::string & basearch() const
	{
	  if ( _basearch.empty() )
	    initArchStr();
	  return _basearch;
	}

	const std::string & releasever() const
	{
	  if( _releasever.empty() )
	    _releasever = Target::distributionVersion( Pathname()/*guess*/ );
	  return _releasever;
	}

      private:
	void initArchStr() const
	{
	  Arch arch( ZConfig::instance().systemArchitecture() );
	  _sysarch = arch.asString();
	  _basearch = arch.baseArch().asString();
	}
      private:
	mutable std::string _sysarch;
	mutable std::string _basearch;
	mutable std::string _releasever;
      };

      /** \brief Replace repo variables on demand
       *
       * Initialisation of repo variables is delayed until they actually occur in
       * a string.
       */
      std::string replacer( const std::string & value_r )
      {
	static ReplacerData _data;

	std::string ret( value_r );
	// Don't need to capture static (non automatic) _data in lambda
	ret = str::replaceAllFun( ret, "$arch",		[]()-> std::string { return _data.sysarch(); } );
	ret = str::replaceAllFun( ret, "$basearch",	[]()-> std::string { return _data.basearch(); } );
	ret = str::replaceAllFun( ret, "$releasever",	[]()-> std::string { return _data.releasever(); } );
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
