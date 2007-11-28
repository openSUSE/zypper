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
    /** Access to the sat-pools string space.
     * Construction from string will place a copy of the string in the
     * string space, if it is not already present.
    */
    class IdStr: protected detail::PoolMember
    {
      friend std::ostream & operator<<( std::ostream & str, const IdStr & obj );

      public:
        /** Default ctor, empty string. */
        IdStr() : _id( Empty.id() ) {}
        /** Ctor from id. */
        explicit IdStr( unsigned id_r ) : _id( id_r ) {}
        /** Ctor from string. */
        explicit IdStr( const char * str_r );
        /** Ctor from string. */
        explicit IdStr( const std::string & str_r );
      public:
        /** No or Null string. */
        static const IdStr Null;
        /** Empty string. */
        static const IdStr::IdStr Empty;
      public:
        bool empty() const
        { return( _id == Empty.id() ); }

      public:
        /** Conversion to <tt>const char *</tt> */
        const char * c_str() const;
        /** Conversion to <tt>std::string</tt> */
        std::string string() const;
        /** \overload */
        std::string asString() const
        { return string(); }

      public:
        /** Expert backdoor. */
        unsigned id() const
        { return _id; }
      private:
        unsigned _id;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates IdStr Stream output */
    std::ostream & operator<<( std::ostream & str, const IdStr & obj );

    /** \relates IdStr */
    inline bool operator==( const IdStr & lhs, const IdStr & rhs )
    { return lhs.id() == rhs.id(); }

    /** \relates IdStr */
    inline bool operator!=( const IdStr & lhs, const IdStr & rhs )
    { return lhs.id() != rhs.id(); }


    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SAT_IDSTR_H
