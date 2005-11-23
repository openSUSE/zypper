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

  File:       ISOLanguage.h

  Author:     Michael Andres <ma@suse.de>
  Maintainer: Michael Andres <ma@suse.de>

  Purpose:

/-*/
#ifndef ISOLanguage_h
#define ISOLanguage_h

#include <iosfwd>
#include <string>

#include <y2util/Rep.h>

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : ISOLanguage
/**
 *
 **/
class ISOLanguage {

  private:

    struct _D;
    VarPtr<_D> _d;

  public:

    ISOLanguage();
    explicit ISOLanguage( const std::string & code_r );
    ~ISOLanguage();

    bool isSet() const;

    std::string code() const;
    std::string name() const;
};

///////////////////////////////////////////////////////////////////

std::ostream & operator<<( std::ostream & str, const ISOLanguage & obj );

///////////////////////////////////////////////////////////////////

inline bool operator==( const ISOLanguage & lhs, const ISOLanguage & rhs ) {
  return( lhs.code() == rhs.code() );
}
inline bool operator==( const std::string & lhs, const ISOLanguage & rhs ) {
  return( lhs == rhs.code() );
}
inline bool operator==( const ISOLanguage & lhs, const std::string & rhs ) {
  return( lhs.code() == rhs );
}

inline bool operator!=( const ISOLanguage & lhs, const ISOLanguage & rhs ) {
  return( ! operator==( lhs, rhs ) );
}
inline bool operator!=( const std::string & lhs, const ISOLanguage & rhs ) {
  return( ! operator==( lhs, rhs ) );
}
inline bool operator!=( const ISOLanguage & lhs, const std::string & rhs ) {
  return( ! operator==( lhs, rhs ) );
}

///////////////////////////////////////////////////////////////////

namespace std {
  template<>
    inline bool less<ISOLanguage>::operator()( const ISOLanguage & lhs,
                                               const ISOLanguage & rhs ) const
    {
      return( lhs.code() < rhs.code() );
    }
}

///////////////////////////////////////////////////////////////////

#endif // ISOLanguage_h
