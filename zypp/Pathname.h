/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/Pathname.h
 *
 * \todo replace by Blocxx
 *
*/

#ifndef ZYPP_PATHNAME_H
#define ZYPP_PATHNAME_H

#include <iosfwd>
#include <string>

namespace zypp {

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : Pathname
//
//	DESCRIPTION :
//
class Pathname {

  private:

    std::string::size_type prfx_i;
    std::string            name_t;

  protected:

    void _assign( const std::string & name_tv );

  public:

    virtual ~Pathname() {}

    Pathname() {
      prfx_i = 0;
      name_t = "";
    }
    Pathname( const Pathname & path_tv ) {
      prfx_i = path_tv.prfx_i;
      name_t = path_tv.name_t;
    }
    Pathname( const std::string & name_tv ) {
      _assign( name_tv );
    }
    Pathname( const char * name_tv ) {
      _assign( name_tv ? name_tv : "" );
    }

    Pathname & operator= ( const Pathname & path_tv );
    Pathname & operator+=( const Pathname & path_tv );

    const std::string & asString() const { return name_t; }

    bool empty()    const { return !name_t.size(); }
    bool absolute() const { return !empty() && name_t[prfx_i] == '/'; }
    bool relative() const { return !empty() && name_t[prfx_i] != '/'; }

    Pathname    dirname()       const { return dirname( *this ); }
    std::string basename()      const { return basename( *this ); }
    Pathname    absolutename()  const { return absolutename( *this ); }
    Pathname    relativename()  const { return relativename( *this ); }

    static Pathname    dirname     ( const Pathname & name_tv );
    static std::string basename    ( const Pathname & name_tv );
    static Pathname    absolutename( const Pathname & name_tv ) { return name_tv.relative() ? cat( "/", name_tv ) : name_tv; }
    static Pathname    relativename( const Pathname & name_tv ) { return name_tv.absolute() ? cat( ".", name_tv ) : name_tv; }

    Pathname        cat( const Pathname & r ) const { return cat( *this, r ); }
    static Pathname cat( const Pathname & l, const Pathname & r );

    Pathname        extend( const std::string & r ) const { return extend( *this, r ); }
    static Pathname extend( const Pathname & l, const std::string & r );

    bool            equal( const Pathname & r ) const { return equal( *this, r ); }
    static bool     equal( const Pathname & l, const Pathname & r );
};

///////////////////////////////////////////////////////////////////

inline bool operator==( const Pathname & l, const Pathname & r ) {
  return Pathname::equal( l, r );
}

inline bool operator!=( const Pathname & l, const Pathname & r ) {
  return !Pathname::equal( l, r );
}

inline Pathname operator+( const Pathname & l, const Pathname & r ) {
  return Pathname::cat( l, r );
}

inline Pathname & Pathname::operator=( const Pathname & path_tv ) {
  if ( &path_tv != this ) {
    prfx_i = path_tv.prfx_i;
    name_t = path_tv.name_t;
  }
  return *this;
}

inline Pathname & Pathname::operator+=( const Pathname & path_tv ) {
  return( *this = *this + path_tv );
}

///////////////////////////////////////////////////////////////////

extern std::ostream & operator<<( std::ostream & str, const Pathname & obj );

///////////////////////////////////////////////////////////////////
} // namespace zypp

#endif // ZYPP_PATHNAME_H
