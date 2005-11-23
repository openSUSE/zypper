/*---------------------------------------------------------------------\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|                               core system                            |
|                                                    (C) SuSE Linux AG |
\----------------------------------------------------------------------/

  File:       ISOCountry.h

  Author:     Michael Andres <ma@suse.de>
  Maintainer: Michael Andres <ma@suse.de>

  Purpose:

/-*/
#ifndef ISOCountry_h
#define ISOCountry_h

#include <iosfwd>
#include <string>

#include <y2util/Rep.h>

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : ISOCountry
/**
 *
 **/
class ISOCountry {

  public:
  private:

    struct _D;
    VarPtr<_D> _d;

  public:

    ISOCountry();
    explicit ISOCountry( const std::string & code_r );
    ~ISOCountry();

    bool isSet() const;

    std::string code() const;
    std::string name() const;
};

///////////////////////////////////////////////////////////////////

std::ostream & operator<<( std::ostream & str, const ISOCountry & obj );

///////////////////////////////////////////////////////////////////

inline bool operator==( const ISOCountry & lhs, const ISOCountry & rhs ) {
  return( lhs.code() == rhs.code() );
}
inline bool operator==( const std::string & lhs, const ISOCountry & rhs ) {
  return( lhs == rhs.code() );
}
inline bool operator==( const ISOCountry & lhs, const std::string & rhs ) {
  return( lhs.code() == rhs );
}

inline bool operator!=( const ISOCountry & lhs, const ISOCountry & rhs ) {
  return( ! operator==( lhs, rhs ) );
}
inline bool operator!=( const std::string & lhs, const ISOCountry & rhs ) {
  return( ! operator==( lhs, rhs ) );
}
inline bool operator!=( const ISOCountry & lhs, const std::string & rhs ) {
  return( ! operator==( lhs, rhs ) );
}

///////////////////////////////////////////////////////////////////

namespace std {
  template<>
    inline bool less<ISOCountry>::operator()( const ISOCountry & lhs,
                                              const ISOCountry & rhs ) const
    {
      return( lhs.code() < rhs.code() );
    }
}

///////////////////////////////////////////////////////////////////

#endif // ISOCountry_h
