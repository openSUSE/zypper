/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
#include <iostream>
#include <fstream>

#include "zypp/base/LogTools.h"
#include "zypp/base/String.h"
#include "zypp/base/Regex.h"

#include "zypp/ZYppFactory.h"
#include "zypp/ZConfig.h"
#include "zypp/Target.h"
#include "zypp/Arch.h"
#include "zypp/repo/RepoVariables.h"
#include "zypp/base/NonCopyable.h"

#define ZYPP_DBG_VAREXPAND 0
#if ( ZYPP_DBG_VAREXPAND )
#warning ZYPP_DBG_VAREXPAND is on
using std::cout;
#endif // ZYPP_DBG_VAREXPAND

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
	{ return ch == '_' || isalnum( ch ); }

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
      class RepoVarsMap : public std::map<std::string,std::string>
      {
      public:
	static RepoVarsMap & instance()
	{ static RepoVarsMap _instance; return _instance; }

	static const std::string * lookup( const std::string & name_r )
	{ return instance()._lookup( name_r ); }

      private:
	const std::string * _lookup( const std::string & name_r )
	{
	  if ( empty() )	// at init / after reset
	  {
	    // load user definitions from vars.d
	    filesystem::dirForEach( ZConfig::instance().systemRoot() / ZConfig::instance().varsPath(),
				    filesystem::matchNoDots(), bind( &RepoVarsMap::parse, this, _1, _2 ) );
	    // releasever_major/_minor are per default derived from releasever.
	    // If releasever is userdefined, inject missing _major/_minor too.
	    deriveFromReleasever( "releasever", /*dont't overwrite user defined values*/false );

	    dumpOn( DBG );
	    // add builtin vars except for releasever{,_major,_minor} (see checkOverride)
	    {
	      const Arch & arch( ZConfig::instance().systemArchitecture() );
	      {
		std::string & var( operator[]( "arch" ) );
		if ( var.empty() ) var = arch.asString();
	      }
	      {
		std::string & var( operator[]( "basearch" ) );
		if ( var.empty() ) var = arch.baseArch().asString();
	      }
	    }
	  }

	  const std::string * ret = checkOverride( name_r );
	  if ( !ret )
	  {
	    // get value from map
	    iterator it = find( name_r );
	    if ( it != end() )
	      ret = &(it->second);
	  }

	  return ret;
	}

	std::ostream & dumpOn( std::ostream & str ) const
	{
	  for ( auto && kv : *this )
	  {
	    str << '{' << kv.first << '=' << kv.second << '}' << endl;
	  }
	  return str;
	}

      private:
	/** Get first line from file */
	bool parse( const Pathname & dir_r, const std::string & str_r )
	{
	  std::ifstream file( (dir_r/str_r).c_str() );
	  operator[]( str_r ) = str::getline( file, /*trim*/false );
	  return true;
	}

	/** Derive \c releasever_major/_minor from \c releasever, keeping or overwrititing existing values. */
	void deriveFromReleasever( const std::string & stem_r, bool overwrite_r )
	{
	  if ( count( stem_r ) )	// releasever is defined..
	  {
	    const std::string & stem_major( stem_r+"_major" );
	    const std::string & stem_minor( stem_r+"_minor" );
	    if ( overwrite_r )
	      splitReleaseverTo( operator[]( stem_r ), &operator[]( stem_major ), &operator[]( stem_minor ) );
	    else
	      splitReleaseverTo( operator[]( stem_r ),
				 count( stem_major ) ? nullptr : &operator[]( stem_major ),
				 count( stem_minor ) ? nullptr : &operator[]( stem_minor ) );
	  }
	}

	/** Split \c releasever at \c '.' and store major/minor parts as requested. */
	void splitReleaseverTo( const std::string & releasever_r, std::string * major_r, std::string * minor_r ) const
	{
	  if ( major_r || minor_r )
	  {
	    std::string::size_type pos = releasever_r.find( "." );
	    if ( pos == std::string::npos )
	    {
	      if ( major_r ) *major_r = releasever_r;
	      if ( minor_r ) minor_r->clear();
	    }
	    else
	    {
	      if ( major_r ) *major_r = releasever_r.substr( 0, pos );
	      if ( minor_r ) *minor_r = releasever_r.substr( pos+1 ) ;
	    }
	  }
	}

	/** Check for conditions overwriting the (user) defined values. */
	const std::string * checkOverride( const std::string & name_r )
	{
	  ///////////////////////////////////////////////////////////////////
	  // Always check for changing releasever{,_major,_minor} (bnc#943563)
	  if ( str::startsWith( name_r, "releasever" )
	    && ( name_r.size() == 10
	      || strcmp( name_r.c_str()+10, "_minor" ) == 0
	      || strcmp( name_r.c_str()+10, "_major" ) == 0 ) )
	  {
	    std::string val( env::ZYPP_REPO_RELEASEVER() );
	    if ( !val.empty() )
	    {
	      // $ZYPP_REPO_RELEASEVER always overwrites any defined value
	      if ( val != operator[]( "$releasever" ) )
	      {
		operator[]( "$releasever" ) = std::move(val);
		deriveFromReleasever( "$releasever", /*overwrite previous values*/true );
	      }
	      return &operator[]( "$"+name_r );
	    }
	    else if ( !count( name_r ) )
	    {
	      // No user defined value, so we follow the target
	      Target_Ptr trg( getZYpp()->getTarget() );
	      if ( trg )
		val = trg->distributionVersion();
	      else
		val = Target::distributionVersion( Pathname()/*guess*/ );

	      if ( val != operator[]( "$_releasever" ) )
	      {
		operator[]( "$_releasever" ) = std::move(val);
		deriveFromReleasever( "$_releasever", /*overwrite previous values*/true );
	      }
	      return &operator[]( "$_"+name_r );
	    }
	    // else:
	    return nullptr;	// get user value from map
	  }
	  ///////////////////////////////////////////////////////////////////

	  return nullptr;	// get user value from map
	}
      };
    } // namespace
    ///////////////////////////////////////////////////////////////////

    std::string RepoVariablesStringReplacer::operator()( const std::string & value ) const
    {
      return RepoVarExpand()( value, RepoVarsMap::lookup );
    }
    std::string RepoVariablesStringReplacer::operator()( std::string && value ) const
    {
      return RepoVarExpand()( value, RepoVarsMap::lookup );
    }

    Url RepoVariablesUrlReplacer::operator()( const Url & value ) const
    {
      static const Url::ViewOptions toReplace = url::ViewOption::DEFAULTS - url::ViewOption::WITH_USERNAME - url::ViewOption::WITH_PASSWORD;
      const std::string & replaced( RepoVarExpand()( value.asString( toReplace ), RepoVarsMap::lookup ) );
      Url newurl;
      if ( !replaced.empty() )
      {
	newurl = replaced;
	newurl.setUsername( value.getUsername( url::E_ENCODED ), url::E_ENCODED );
	newurl.setPassword( value.getPassword( url::E_ENCODED ), url::E_ENCODED );
      }
      return newurl;
    }
  } // namespace repo
  ///////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace zyppintern
{
  using namespace zypp;
  // internal helper called when re-acquiring the lock
  void repoVariablesReset()
  { repo::RepoVarsMap::instance().clear(); }

} // namespace zyppintern
///////////////////////////////////////////////////////////////////
