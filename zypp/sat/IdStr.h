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
     *
     * Construction from string will place a copy of the string in the
     * string space, if it is not already present.
     *
     * While comparison differs between \ref IdStr::Null and \ref IdStr::Empty
     * ( \c NULL and \c "" ), both are represented by an empty string \c "".
    */
    class IdStr: protected detail::PoolMember,
                 private base::SafeBool<IdStr>
    {
      public:
        /** Default ctor, empty string. */
        IdStr() : _id( 1 ) {}

        /** Ctor from id. */
        explicit IdStr( detail::IdType id_r ) : _id( id_r ) {}

        /** Ctor from string. */
        explicit IdStr( const char * str_r );

        /** Ctor from string. */
        explicit IdStr( const std::string & str_r );

      public:
        /** No or Null string ( Id \c 0 ). */
        static const IdStr Null;

        /** Empty string. */
        static const IdStr Empty;

      public:
        /** Evaluate in a boolean context <tt>( != \c Null )</tt>. */
        using base::SafeBool<IdStr>::operator bool_type;

        /** Whether the string is empty.
         * This is true for \ref Null and \ref Empty.
        */
        bool empty() const
        { return( _id == 1 || _id == 0 ); }

        /** The strings size. */
        unsigned size() const;

      public:
        /** Conversion to <tt>const char *</tt> */
        const char * c_str() const;

        /** Conversion to <tt>std::string</tt> */
        std::string string() const
        { return c_str(); }

        /** \overload */
        std::string asString() const
        { return c_str(); }

      public:
        /** Fast compare equal. */
        bool compareEQ( const IdStr & rhs ) const
        { return( _id == rhs.id() ); }

        /** Compare IdStr returning <tt>-1,0,1</tt>. */
        int compare( const IdStr & rhs ) const;

        /** \overload */
        int compare( const char * rhs ) const;

        /** \overload */
        int compare( const std::string & rhs ) const
        { return compare( rhs.c_str() ); }

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

    /** \relates IdStr Equal */
    inline bool operator==( const IdStr & lhs, const IdStr & rhs )
    { return lhs.compareEQ( rhs ); }
    /** \overload */
    inline bool operator==( const IdStr & lhs, const char * rhs )
    { return lhs.compare( rhs ) == 0; }
    /** \overload */
    inline bool operator==( const IdStr & lhs, const std::string & rhs )
    { return lhs.compare( rhs ) == 0; }
    /** \overload */
    inline bool operator==( const char * lhs, const IdStr & rhs )
    { return rhs.compare( lhs ) == 0; }
    /** \overload */
    inline bool operator==( const std::string & lhs, const IdStr & rhs )
    { return rhs.compare( lhs ) == 0; }

    /** \relates IdStr NotEqual */
    inline bool operator!=( const IdStr & lhs, const IdStr & rhs )
    { return ! lhs.compareEQ( rhs ); }
    /** \overload */
    inline bool operator!=( const IdStr & lhs, const char * rhs )
    { return lhs.compare( rhs ) != 0; }
    /** \overload */
    inline bool operator!=( const IdStr & lhs, const std::string & rhs )
    { return lhs.compare( rhs ) != 0; }
    /** \overload */
    inline bool operator!=( const char * lhs, const IdStr & rhs )
    { return rhs.compare( lhs ) != 0; }
    /** \overload */
    inline bool operator!=( const std::string & lhs, const IdStr & rhs )
    { return rhs.compare( lhs ) != 0; }

    /** \relates IdStr Less */
    inline bool operator<( const IdStr & lhs, const IdStr & rhs )
    { return lhs.compare( rhs ) < 0; }
    /** \overload */
    inline bool operator<( const IdStr & lhs, const char * rhs )
    { return lhs.compare( rhs ) < 0; }
    /** \overload */
    inline bool operator<( const IdStr & lhs, const std::string & rhs )
    { return lhs.compare( rhs ) < 0; }
    /** \overload */
    inline bool operator<( const char * lhs, const IdStr & rhs )
    { return rhs.compare( lhs ) >= 0; }
    /** \overload */
    inline bool operator<( const std::string & lhs, const IdStr & rhs )
    { return rhs.compare( lhs ) >= 0; }

    /** \relates IdStr LessEqual*/
    inline bool operator<=( const IdStr & lhs, const IdStr & rhs )
    { return lhs.compare( rhs ) <= 0; }
    /** \overload */
    inline bool operator<=( const IdStr & lhs, const char * rhs )
    { return lhs.compare( rhs ) <= 0; }
    /** \overload */
    inline bool operator<=( const IdStr & lhs, const std::string & rhs )
    { return lhs.compare( rhs ) <= 0; }
    /** \overload */
    inline bool operator<=( const char * lhs, const IdStr & rhs )
    { return rhs.compare( lhs ) > 0; }
    /** \overload */
    inline bool operator<=( const std::string & lhs, const IdStr & rhs )
    { return rhs.compare( lhs ) > 0; }

     /** \relates IdStr Greater */
    inline bool operator>( const IdStr & lhs, const IdStr & rhs )
    { return lhs.compare( rhs ) > 0; }
    /** \overload */
    inline bool operator>( const IdStr & lhs, const char * rhs )
    { return lhs.compare( rhs ) > 0; }
    /** \overload */
    inline bool operator>( const IdStr & lhs, const std::string & rhs )
    { return lhs.compare( rhs ) > 0; }
    /** \overload */
    inline bool operator>( const char * lhs, const IdStr & rhs )
    { return rhs.compare( lhs ) <= 0; }
    /** \overload */
    inline bool operator>( const std::string & lhs, const IdStr & rhs )
    { return rhs.compare( lhs ) <= 0; }

    /** \relates IdStr GreaterEqual */
    inline bool operator>=( const IdStr & lhs, const IdStr & rhs )
    { return lhs.compare( rhs ) >= 0; }
    /** \overload */
    inline bool operator>=( const IdStr & lhs, const char * rhs )
    { return lhs.compare( rhs ) >= 0; }
    /** \overload */
    inline bool operator>=( const IdStr & lhs, const std::string & rhs )
    { return lhs.compare( rhs ) >= 0; }
    /** \overload */
    inline bool operator>=( const char * lhs, const IdStr & rhs )
    { return rhs.compare( lhs ) < 0; }
    /** \overload */
    inline bool operator>=( const std::string & lhs, const IdStr & rhs )
    { return rhs.compare( lhs ) < 0; }

   /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////

  /** Drag into namespace zypp*/
  using sat::IdStr;

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SAT_IDSTR_H
