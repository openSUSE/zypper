/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/Locale.cc
 *
*/
#include <iostream>
#include <map>

#include "zypp/Locale.h"
#include "zypp/ZConfig.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{
  /** Wrap static codemap data. */
  struct CodeMaps
  {
   /** Return IdString without trailing garbage. */
    static IdString withoutTrash( IdString code_r )
    {
       boost::string_ref::size_type sep = trashStart( code_r );
       if ( sep != boost::string_ref::npos )
	 code_r = IdString( code_r.c_str(), sep );
       return code_r;
    }

    /** Return IdString without trailing garbage. */
    static IdString withoutTrash( const std::string & code_r )
    { return withoutTrash( boost::string_ref(code_r) ); }

    /** Return IdString without trailing garbage. */
    static IdString withoutTrash( const char * code_r )
    { return( code_r ? withoutTrash( boost::string_ref(code_r) ) : IdString::Null ); }

    /** Return IdString from language/country codes. */
    static IdString combineLC( LanguageCode language_r, CountryCode country_r )
    {
      IdString ret;
      if ( language_r )
      {
	if ( country_r )
	  ret = IdString( std::string(language_r) + "_" + country_r.c_str() );
	else
	  ret = IdString(language_r);
      }
      else
      {
	if ( country_r )
	  ret = IdString( "_" + std::string(country_r) );
	else if ( ! ( IdString(language_r) || IdString(country_r) ) )
	  ret = IdString::Null;
	// else IdString::Empty
      }
      return ret;
    }

    /** The singleton */
    static CodeMaps & instance()
    {
      static CodeMaps _instance;
      return _instance;
    }

    LanguageCode language( IdString index_r )
    { return getIndex( index_r )._l; }

    CountryCode country( IdString index_r )
    { return  getIndex( index_r )._c; }

    std::string name( IdString index_r )
    {
      const LC & lc( getIndex( index_r ) );
      std::string ret( lc._l.name() );
      if ( lc._c )
      {
	ret += " (";
	ret += lc._c.name();
	ret += ")";
      }
      return ret;
    }

    Locale fallback( IdString index_r )
    {
      static const IdString special( "pt_BR" );
      Locale ret;
      if ( index_r == special )	// "pt_BR"->"en" - by now the only fallback exception
	ret = Locale::enCode;
      else
      {
	const LC & lc( getIndex( index_r ) );
	if ( lc._c )
	  ret = lc._l;
	else if ( lc._l && lc._l != LanguageCode::enCode )
	  ret = Locale::enCode;
      }
      return ret;
    }

  private:
    static IdString withoutTrash( boost::string_ref code_r )
    {
      boost::string_ref::size_type sep = trashStart( code_r );
      if ( sep != boost::string_ref::npos )
	code_r = code_r.substr( 0, sep );
      return IdString( code_r );
    }

    static boost::string_ref::size_type trashStart( boost::string_ref code_r )
    { return code_r.find_first_of( "@." ); }

    static boost::string_ref::size_type trashStart( IdString code_r )
    { return trashStart( boost::string_ref(code_r.c_str()) ); }

  private:
    struct LC {
      LC()					{}
      LC( LanguageCode l_r )			: _l( l_r ) {}
      LC( LanguageCode l_r, CountryCode c_r )	: _l( l_r ), _c( c_r ) {}
      LanguageCode _l;
      CountryCode  _c;
    };
    typedef std::unordered_map<IdString,LC> CodeMap;

    /** Ctor initializes the code maps. */
    CodeMaps()
    : _codeMap( { { IdString::Null,  LC( LanguageCode(IdString::Null),  CountryCode(IdString::Null) )  }
                , { IdString::Empty, LC( LanguageCode(IdString::Empty), CountryCode(IdString::Empty) ) } } )
    {}

    /** Return \ref LC for \a index_r, creating it if necessary. */
    const LC & getIndex( IdString index_r )
    {
      auto it = _codeMap.find( index_r );
      if ( it == _codeMap.end() )
      {
	CodeMap::value_type newval( index_r, LC() );

	boost::string_ref str( index_r.c_str() );
	boost::string_ref::size_type sep = str.find( '_' );
	if ( sep == boost::string_ref::npos )
	  newval.second._l = LanguageCode( IdString(index_r) );
	else
	{
	  newval.second._l = LanguageCode( IdString(str.substr( 0, sep )) );
	  newval.second._c = CountryCode( IdString(str.substr( sep+1 )) );
	}

	it = _codeMap.insert( std::move(newval) ).first;
      }
      return it->second;
    }

  private:
    CodeMap _codeMap;
  };

  ///////////////////////////////////////////////////////////////////
  // class Locale
  ///////////////////////////////////////////////////////////////////

  const Locale Locale::noCode;
  const LanguageCode LanguageCode::enCode("en");	// from in LanguageCode.cc as Locale::enCode depends on it
  const Locale Locale::enCode( LanguageCode::enCode );

  Locale::Locale()
  {}

  Locale::Locale( IdString str_r )
  : _str( CodeMaps::withoutTrash( str_r ) )
  {}

  Locale::Locale( const std::string & str_r )
  : _str( CodeMaps::withoutTrash( str_r ) )
  {}

  Locale::Locale( const char * str_r )
  : _str( CodeMaps::withoutTrash( str_r ) )
  {}

  Locale::Locale( LanguageCode language_r, CountryCode country_r )
  : _str( CodeMaps::combineLC( language_r, country_r ) )
  {}

  Locale::~Locale()
  {}

  LanguageCode Locale::language() const
  { return CodeMaps::instance().language( _str ); }

  CountryCode Locale::country() const
  { return CodeMaps::instance().country( _str ); }

  std::string Locale::name() const
  { return CodeMaps::instance().name( _str ); }

  Locale Locale::fallback() const
  { return CodeMaps::instance().fallback( _str ); }

  ///////////////////////////////////////////////////////////////////

  Locale Locale::bestMatch( const LocaleSet & avLocales_r, Locale requested_r )
  {
    if ( ! avLocales_r.empty() )
    {
      if ( ! requested_r )
	requested_r = ZConfig::instance().textLocale();
      for ( ; requested_r; requested_r = requested_r.fallback() )
      {
        if ( avLocales_r.count( requested_r ) )
          return requested_r;
      }
    }
    return Locale();
  }

} // namespace zypp
///////////////////////////////////////////////////////////////////
