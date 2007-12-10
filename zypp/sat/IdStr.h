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

#include "zypp/base/SafeBool.h"

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
    class IdStr: protected detail::PoolMember,
                 private base::SafeBool<IdStr>
    {
      public:
        /** Default ctor, empty string. */
        IdStr() : _id( Empty.id() ) {}
        /** Ctor from id. */
        explicit IdStr( detail::IdType id_r ) : _id( id_r ) {}
        /** Ctor from string. */
        explicit IdStr( const char * str_r );
        /** Ctor from string. */
        explicit IdStr( const std::string & str_r );
        /** Evaluate in a boolean context <tt>( != \c Null )</tt>. */
        using base::SafeBool<IdStr>::operator bool_type;
      public:
        /** No or Null string ( Id \c 0 ). */
        static const IdStr Null;
        /** Empty string. */
        static const IdStr Empty;
      public:
        /** Whether string is empty. */
        bool empty() const
        { return( _id == Empty.id() ); }
        /** the strings size. */
        unsigned size() const;
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
        detail::IdType id() const
        { return _id; }
      private:
        friend base::SafeBool<IdStr>::operator bool_type() const;
        bool boolTest() const { return _id; }
      private:
        detail::IdType _id;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates IdStr Stream output */
    std::ostream & operator<<( std::ostream & str, const IdStr & obj );

    /** \relates IdStr */
    inline bool operator==( const IdStr & lhs, const IdStr & rhs )
    { return lhs.id() == rhs.id(); }
    /** \overload */
    inline bool operator==( const IdStr & lhs, const char * rhs )
    { if ( ! rhs ) return( ! lhs ); return( ::strcmp( lhs.c_str(), rhs ) == 0 ); }
    /** \overload */
    inline bool operator==( const IdStr & lhs, const std::string & rhs )
    { return( lhs == rhs.c_str() ); }
    /** \overload */
    inline bool operator==( const char * lhs, const IdStr & rhs )
    { return( rhs == lhs ); }
    /** \overload */
    inline bool operator==( const std::string & lhs, const IdStr & rhs )
    { return( rhs == lhs ); }

    /** \relates IdStr */
    inline bool operator!=( const IdStr & lhs, const IdStr & rhs )
    { return ! ( lhs == rhs ); }
    /** \overload */
    inline bool operator!=( const IdStr & lhs, const char * rhs )
    { return ! ( lhs == rhs ); }
    /** \overload */
    inline bool operator!=( const IdStr & lhs, const std::string & rhs )
    { return ! ( lhs == rhs ); }
    /** \overload */
    inline bool operator!=( const char * lhs, const IdStr & rhs )
    { return ! ( lhs == rhs ); }
    /** \overload */
    inline bool operator!=( const std::string & lhs, const IdStr & rhs )
    { return ! ( lhs == rhs ); }

    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SAT_IDSTR_H
