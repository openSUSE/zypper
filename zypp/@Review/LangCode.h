/*---------------------------------------------------------------------\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|                               core system                            |
|                                                        (C) SuSE GmbH |
\----------------------------------------------------------------------/

   File:       LangCode.h

   Author:     Michael Andres <ma@suse.de>
   Maintainer: Michael Andres <ma@suse.de>

/-*/
#ifndef LangCode_h
#define LangCode_h

#include <iosfwd>

#include <y2util/ISOLanguage.h>
#include <y2util/ISOCountry.h>

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : LangCode
/**
 * Store ISO <code>language[_country]</code> codes.
 **/
class LangCode {

  private:

    ISOLanguage _language;
    ISOCountry  _country;

  public:

    LangCode() {}

    explicit LangCode( const std::string & code_r );

    LangCode( const ISOLanguage & language_r,
	      const ISOCountry & country_r = ISOCountry() )
      : _language( language_r )
      , _country( country_r )
    {}

    ~LangCode() {}

    bool isSet() const { return( _language.isSet() || _country.isSet() ); }

    bool hasLanguage() const { return _language.isSet(); }
    bool hasCountry() const { return _country.isSet(); }

    std::string code() const;
    std::string languageCode() const { return _language.code(); }
    std::string countryCode() const { return _country.code(); }

    std::string name() const;
    std::string languageName() const { return _language.name(); }
    std::string countryName() const { return _country.name(); }

  public:

    ISOLanguage language() const { return _language; }
    ISOCountry country() const { return _country; }
};

///////////////////////////////////////////////////////////////////

std::ostream & operator<<( std::ostream & str, const LangCode & obj );

///////////////////////////////////////////////////////////////////

inline bool operator==( const LangCode & lhs, const LangCode & rhs ) {
  return( lhs.code() == rhs.code() );
}
inline bool operator==( const std::string & lhs, const LangCode & rhs ) {
  return( lhs == rhs.code() );
}
inline bool operator==( const LangCode & lhs, const std::string & rhs ) {
  return( lhs.code() == rhs );
}

inline bool operator!=( const LangCode & lhs, const LangCode & rhs ) {
  return( ! operator==( lhs, rhs ) );
}
inline bool operator!=( const std::string & lhs, const LangCode & rhs ) {
  return( ! operator==( lhs, rhs ) );
}
inline bool operator!=( const LangCode & lhs, const std::string & rhs ) {
  return( ! operator==( lhs, rhs ) );
}

///////////////////////////////////////////////////////////////////


namespace std {
  template<>
    inline bool less<LangCode>::operator()( const LangCode & lhs,
                                            const LangCode & rhs ) const
    {
      if ( less<ISOLanguage>()( lhs.language(), rhs.language() ) )
        return true;
      if ( less<ISOLanguage>()( rhs.language(), lhs.language() ) )
        return false;
      return less<ISOCountry>()( lhs.country(), rhs.country() );
    }
}

///////////////////////////////////////////////////////////////////

#endif // LangCode_h
