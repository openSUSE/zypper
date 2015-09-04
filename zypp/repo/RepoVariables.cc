/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
#include <cstring>

#define ZYPP_DBG_VAREXPAND 0
#if ( ZYPP_DBG_VAREXPAND )
#warning ZYPP_DBG_VAREXPAND is on
#include <iostream>
#include <sstream>
using std::cout;
using std::endl;
#endif // ZYPP_DBG_VAREXPAND

#include "zypp/base/LogTools.h"
#include "zypp/base/String.h"
#include "zypp/base/Regex.h"

#include "zypp/ZYppFactory.h"
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
    // RepoVarExpand
    ///////////////////////////////////////////////////////////////////
    namespace
    {
      ///////////////////////////////////////////////////////////////////
      /// \class FindVar
      /// \brief Helper scanning for variable definitions in a string
      ///////////////////////////////////////////////////////////////////
      struct FindVar
      {
	bool _embedded;		///< A (formerly) embedded string may have esacped \c $, \c closebrace and \c backslash
	const char * _sbeg;	///< start of string to scan
	const char * _vbeg;	///< [$]{variable:-word} / [$]{variable} / if embedded also on [\\]
	const char * _nbeg;	///< ${[v]ariable:-word} / ${[v]ariable}
	const char * _nend;	///< ${variable[:]-word} / ${variable[}]
	const char * _vend;	///< ${variable:-word}[] / ${variable}[]
	const char * _send;	///< end of scan (next $ or nullptr if done)

	FindVar( const std::string & str_r, bool embedded_r )
	: _embedded( embedded_r )
	, _sbeg( str_r.c_str() )
	, _vbeg( nullptr )
	, _nbeg( nullptr )
	, _nend( nullptr )
	, _vend( nullptr )
	, _send( findVarStart( _sbeg ) )
	{}

	/** Nullptr in _send indicates we scanned the whole string. */
	bool done() const
	{ return !_send; }

	/** Advance to first/next var if there is one */
	bool nextVar()
	{
	  if ( done() )
	    return false;

	  do {
	    if ( _vbeg && !_vend )	// loop internal: no findVarEnd at current $; skip it
	      _send = findVarStart( _vbeg+1 );
	    _vbeg = _send;		// next $ or null if string end
	    _nbeg = _nend = _vend = _send = nullptr;
	    if ( ! _vbeg )		// done!
	      return false;
	  } while( ! findVarEnd() );

	  return true;
	}

	/** Valid _vend indicates valid var data in scan. */
	bool hasVar() const
	{ return _vend; }

	//
	// Methods below are only valid if hasVar() == true
	//

	/** Return the full var text */
	std::string var() const
	{ return std::string( _vbeg, _vend ); }

	/** Return the var name */
	std::string varName() const
	{ return std::string( _nbeg, _nend ); }

	/** Whether this is a conditional var (${..:[+-]...}) */
	bool varIsConditional() const
	{ return( *(_vbeg+1) == '{' && *_nend == ':' ); }

	/** The var type: \c \, \c $, \c - , \c +
	* \li \c \ backslash escaped literal
	* \li \c $	plain variable
	* \li \c - conditional: default value
	* \li \c + conditional: alternate value
	*/
	int varType() const
	{ return( varIsConditional() ? *(_nend+1) : *_vbeg ); }

	/** Return embedded value in conditional vars or empty string */
	std::string varEmbedded() const
	{ return( varIsConditional() ? std::string( _nend+2, _vend-1 ) : std::string() ); }


	/** Have unwritten data before var? */
	bool hasVarPrefix() const
	{ return ( _sbeg != _vbeg ); }

	/** Return unwritten data before var */
	std::string varPrefix() const
	{ return std::string( _sbeg, _vbeg ); }

	/** Indicate all data up to _vend were written */
	void wroteVar()
	{ _sbeg = _vend; }

      private:
	/** Return next \c $ */
	const char * findVarStart( const char * sbeg_r ) const
	{
	  for ( ; *sbeg_r; ++sbeg_r )
	    if ( *sbeg_r == '$' || ( _embedded && *sbeg_r == '\\' ) )
	      return sbeg_r;
	  return nullptr;
	}

	/** Valid var name char */
	bool isnamech( int ch ) const
	{ return ch == '_' || isalpha( ch ); }

	/** Scan for a valid variable starting at _vbeg (storing the values) */
	bool findVarEnd()
	{
	  // asserted: *_vbeg == '$' || '\\'
	  if ( ! findVarEnd( _vbeg, _nbeg, _nend, _vend ) )
	    return false;
	  _send = findVarStart( _vend );
	  return true;
	}

	/** Skip over valid variable starting at vbeg (return end in \a vend). */
	const char * findVarEnd( const char * vbeg ) const
	{
	  // asserted: *_vbeg == '$'
	  const char * nbeg = nullptr;
	  const char * nend = nullptr;
	  const char * vend = nullptr;
	  findVarEnd( vbeg, nbeg, nend, vend );
	  return vend;
	}

	/** Scan for a valid variable starting at vbeg (const version returning the values). */
	bool findVarEnd( const char * vbeg, const char *& nbeg, const char *& nend, const char *& vend ) const
	{
	  // embedded only: handle backslash escaped chars
	  if ( *_vbeg == '\\' )
	  {
	    nbeg = vbeg+1;
	    if ( *nbeg == '$'
	      || *nbeg == '}'
	      || *nbeg == '\\' )
	    {
	      nend = vend = vbeg+2;
	      return true;
	    }
	    return false;
	  }

	  // asserted: *vbeg == '$'
	  // vbeg: [$]{variable:-word} / [$]{variable}
	  // nbeg: ${[v]ariable:-word} / ${[v]ariable}
	  bool braced = ( *(vbeg+1) == '{' ); //}
	  nbeg = vbeg+( braced ? 2 : 1 );
	  if ( !isnamech( *nbeg ) )	// don't allow empty var name
	    return false;
	  for ( nend = nbeg+1; isnamech( *nend ); ++nend )
	  {;} // skip over var name
	  // nend: ${variable[:]-word} / ${variable[}]

	  // vend: ${variable:-word}[] / ${variable}[]
	  // stay with ( vend == nullptr ) until you know it's valid
	  if ( braced )
	  {
	    if ( *nend == '}' )
	    {
	      vend = nend+1;
	    }
	    else if ( *nend == ':' )
	    {
	      const char * scan = nend+1;
	      if ( *scan == '+' || *scan == '-' )
	      {
		++scan;
		// find first not escaped '}'
		while ( *scan )
		{
		  if ( *scan == '\\' )
		  {
		    ++scan;	// next char is skipped
		    if ( *scan )
		      ++scan;
		  }
		  else if ( *scan == '$' )
		  {
		    // an embedded var?
		    if ( ! (scan = findVarEnd( scan )) )
		      return false;
		  }
		  else if ( *scan == '}' )
		  {
		    vend = scan+1;	// ==> unesacped '}', we're done!
		    break;
		  }
		  else
		    ++scan;	// literal
		}
		// ( ! *scan ) => end of string while looking for unesacped '}'
	      }
	      else
		; // err: ':' not followed by '+' or '-'
	    }
	    else
	      ; // err: braced name must end with '}' or ':'
	  }
	  else
	  {
	    vend = nend;	// un-braced
	  }
	  return( vend != nullptr );
	}
      };

      bool _expand( std::string &, const std::string & value_r, unsigned level_r, RepoVarExpand::VarRetriever & varRetriever_r );

      inline std::string expand( const std::string & value_r, unsigned level_r, RepoVarExpand::VarRetriever & varRetriever_r )
      {
	std::string ret;
	if ( ! _expand( ret, value_r, level_r, varRetriever_r ) )
	  ret = value_r;
	return ret;
      }

      inline std::string expand( std::string && value_r, unsigned level_r, RepoVarExpand::VarRetriever & varRetriever_r )
      {
	std::string ret;
	if ( ! _expand( ret, value_r, level_r, varRetriever_r ) )
	  ret = std::move(value_r);
	return ret;
      }

      /** Expand variables in \a value_r depending on \a level-r
      * <tt>level_r > 0</tt> may have escaped chars outside braces.
      */
      inline bool _expand( std::string & result_r, const std::string & value_r, unsigned level_r, RepoVarExpand::VarRetriever & varRetriever_r )
      {
#if ( ZYPP_DBG_VAREXPAND )
	cout << std::string(  2*level_r, ' ' ) << "\033[7m>>" << value_r << "<<\033[27m" << endl;
	std::ostringstream dbg;
	const char * dbgsbeg = value_r.c_str();	// track vars we already added to dbg
	unsigned dbgi = 0;			// color 1-5 var / 6 moved value_r
	dbg << std::string(  2*level_r, ' ' ) << ">>";
#endif // ZYPP_DBG_VAREXPAND

	bool expanded = false;

	if ( ! value_r.empty() )
	{
	  FindVar scan( value_r, level_r );	// level_r > 0 is embedded
	  while ( scan.nextVar() )
	  {
	    static const std::string _emptyValue;
	    const std::string *const knownVar = ( varRetriever_r ? varRetriever_r( scan.varName() ) : nullptr );
	    const std::string & varValue( knownVar ? *knownVar : _emptyValue );

#if ( ZYPP_DBG_VAREXPAND )
	    dbg << std::string(dbgsbeg,scan._vbeg) << "\033[3" << ((dbgi%5)+1) << "m" << scan.var() << "\033[0m";
	    cout << dbg.str() << "|<< " << scan.varName() << " " << (knownVar?"("+varValue+")":"-") << " {" << scan.varEmbedded() << "}" << endl;
	    dbgsbeg = scan._vend;
	    dbgi++;
#endif // ZYPP_DBG_VAREXPAND

	    bool mustSubstitute = false;	// keep original text per default
	    std::string substitutionValue;

	    int varType = scan.varType();
	    if ( varType == '$' )	// plain var
	    {
	      if ( knownVar )
	      {
		mustSubstitute = true;
		substitutionValue = varValue;
	      }
	      else
		; // keep original text per default
	    }
	    else if ( varType == '-' ) // ':-' default value
	    {
	      mustSubstitute = true;
	      if ( varValue.empty() )
		substitutionValue = expand( scan.varEmbedded(), level_r+1, varRetriever_r );
	      else
		substitutionValue = varValue;
	    }
	    else if ( varType == '+' ) // ':+' alternate value
	    {
	      mustSubstitute = true;
	      if ( ! varValue.empty() )
		substitutionValue = expand( scan.varEmbedded(), level_r+1, varRetriever_r );
	      else
		; // empty substitutionValue
	    }
	    else if ( varType == '\\' ) // backslash escaped literal (in varName)
	    {
	      mustSubstitute = true;
	      substitutionValue = scan.varName();
	    }
	    else
	      ; // keep original text per default

	    if ( mustSubstitute  )
	    {
	      if ( scan.hasVarPrefix() )
		result_r += scan.varPrefix();
	      if ( ! substitutionValue.empty() )
		result_r += substitutionValue;
	      scan.wroteVar(); // this moves scan._sbeg so we can later see what's already written
	    }
	  }

#if ( ZYPP_DBG_VAREXPAND )
	  dbg << std::string( dbgsbeg ) << (scan._sbeg == value_r.c_str() ? "<<\033[36m(moved)\033[0m" : "");
#endif // ZYPP_DBG_VAREXPAND

	  // handle unwritten data:
	  if ( scan._sbeg != value_r.c_str() )
	  {
	    expanded = true;
	    if ( *scan._sbeg )
	      result_r += std::string( scan._sbeg );
	  }
	  else
	    ; // no replacements at all
	}

#if ( ZYPP_DBG_VAREXPAND )
	dbg << "<<";
	cout << dbg.str() << endl;
	cout << std::string(  2*level_r, ' ' ) << "\033[36m->" << result_r << "<-\033[0m" << endl;
#endif // ZYPP_DBG_VAREXPAND
	return expanded;
      }
    } // namespace
    ///////////////////////////////////////////////////////////////////

    std::string RepoVarExpand::operator()( const std::string & value_r, VarRetriever varRetriever_r ) const
    { return expand( value_r, 0, varRetriever_r ); }

    std::string RepoVarExpand::operator()( std::string && value_r, VarRetriever varRetriever_r ) const
    { return expand( std::move(value_r), 0, varRetriever_r ); }

    ///////////////////////////////////////////////////////////////////
    // RepoVariables*Replace
    ///////////////////////////////////////////////////////////////////
    namespace
    {
      inline std::string getReleaseverString()
      {
	std::string ret( env::ZYPP_REPO_RELEASEVER() );
	if( ret.empty() )
	{
	  Target_Ptr trg( getZYpp()->getTarget() );
	  if ( trg )
	    ret = trg->distributionVersion();
	  else
	    ret = Target::distributionVersion( Pathname()/*guess*/ );
	}
	else
	  WAR << "ENV overwrites $releasever=" << ret << endl;

	return ret;
      }

      /** \brief Provide lazy initialized repo variables
       */
      struct RepoVars : private zypp::base::NonCopyable
      {
	typedef const std::string & (RepoVars::*Getter)() const;

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
	  // check for changing releasever (bnc#943563)
	  std::string check( getReleaseverString() );
	  if ( check != _releasever )
	  {
	    _releasever = std::move(check);
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

      /** \brief */
      const std::string * repoVarLookup( const std::string & name_r )
      {
	RepoVars::Getter getter = nullptr;
	switch ( name_r.size() )
	{
#define ASSIGN_IF(NAME,GETTER) if ( name_r == NAME ) getter = GETTER
	  case  4:	ASSIGN_IF( "arch",		&RepoVars::arch );		break;
	  case  8:	ASSIGN_IF( "basearch",		&RepoVars::basearch );		break;
	  case 10:	ASSIGN_IF( "releasever",	&RepoVars::releasever );	break;
	  case 16:	ASSIGN_IF( "releasever_major",	&RepoVars::releaseverMajor );
	      else	ASSIGN_IF( "releasever_minor",	&RepoVars::releaseverMinor );	break;
#undef ASSIGN_IF
	}

	const std::string * ret = nullptr;
	if ( getter )	// known var
	{
	  static const RepoVars _repoVars;
	  ret = &(_repoVars.*getter)();
	}
	return ret;
      }
    } // namespace
    ///////////////////////////////////////////////////////////////////

    std::string RepoVariablesStringReplacer::operator()( const std::string & value ) const
    {
      return RepoVarExpand()( value, repoVarLookup );
    }
    std::string RepoVariablesStringReplacer::operator()( std::string && value ) const
    {
      return RepoVarExpand()( value, repoVarLookup );
    }

    Url RepoVariablesUrlReplacer::operator()( const Url & value ) const
    {
      RepoVarExpand expand;
      Url newurl( value );
      newurl.setPathData( expand( value.getPathData(), repoVarLookup ) );
      newurl.setQueryString( expand( value.getQueryString(), repoVarLookup ) );
      return newurl;
    }

  } // namespace repo
  ///////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
