/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/IdStr.h
 *
*/
#ifndef ZYPP_SAT_IDSTR_H
#define ZYPP_SAT_IDSTR_H

#include <iosfwd>
#include <string>

#include "zypp/sat/detail/PoolMember.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : IdStr
    //
    /** */
    class IdStr: protected detail::PoolMember
    {
      friend std::ostream & operator<<( std::ostream & str, const IdStr & obj );

      public:
        IdStr() : _id( 0 ) {}
        explicit IdStr( int id_r ) : _id( id_r ) {}
        explicit IdStr( const char * str_r );
        explicit IdStr( const std::string & str_r );
      public:
        static const IdStr Null;
      public:
        const char * c_str() const;
        std::string string() const;
      public:
        int get() const { return _id; }
      private:
        int _id;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates IdStr Stream output */
    std::ostream & operator<<( std::ostream & str, const IdStr & obj );

    /** \relates IdStr */
    inline bool operator==( const IdStr & lhs, const IdStr & rhs )
    { return lhs.get() == rhs.get(); }

    /** \relates IdStr */
    inline bool operator!=( const IdStr & lhs, const IdStr & rhs )
    { return lhs.get() != rhs.get(); }


    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SAT_IDSTR_H
