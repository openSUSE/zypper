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
        /** Compare IdStr returning <tt>-1,0,1</tt>. */
        int compare( const IdStr & rhs ) const;
        /** \overload */
        int compare( const char * rhs ) const;
        /** \overload */
        int compare( const std::string & rhs ) const
        { return compare( rhs.c_str() ); }

        /** Compare IdStr returning <tt>-1,0,1</tt>. */
        static int compare( const IdStr & lhs, const IdStr & rhs )
        { return lhs.compare( rhs ); }
        /** \overload */
        static int compare( const IdStr & lhs, const char * rhs )
        { return lhs.compare( rhs ); }
        /** \overload */
        static int compare( const IdStr & lhs, const std::string & rhs )
        { return lhs.compare( rhs ); }
        /** \overload */
        static int compare( const char * lhs, const IdStr & rhs )
        { return -rhs.compare( lhs ); }
        /** \overload */
        static int compare( const std::string & lhs, const IdStr & rhs )
        { return -rhs.compare( lhs ); }
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
    { return lhs.id() == rhs.id(); }
    /** \overload */
    inline bool operator==( const IdStr & lhs, const char * rhs )
    { return IdStr::compare( lhs, rhs ) == 0; }
    /** \overload */
    inline bool operator==( const IdStr & lhs, const std::string & rhs )
    { return IdStr::compare( lhs, rhs ) == 0; }
    /** \overload */
    inline bool operator==( const char * lhs, const IdStr & rhs )
    { return IdStr::compare( lhs, rhs ) == 0; }
    /** \overload */
    inline bool operator==( const std::string & lhs, const IdStr & rhs )
    { return IdStr::compare( lhs, rhs ) == 0; }

    /** \relates IdStr NotEqual */
    inline bool operator!=( const IdStr & lhs, const IdStr & rhs )
    { return lhs.id() != rhs.id(); }
    /** \overload */
    inline bool operator!=( const IdStr & lhs, const char * rhs )
    { return IdStr::compare( lhs, rhs ) != 0; }
    /** \overload */
    inline bool operator!=( const IdStr & lhs, const std::string & rhs )
    { return IdStr::compare( lhs, rhs ) != 0; }
    /** \overload */
    inline bool operator!=( const char * lhs, const IdStr & rhs )
    { return IdStr::compare( lhs, rhs ) != 0; }
    /** \overload */
    inline bool operator!=( const std::string & lhs, const IdStr & rhs )
    { return IdStr::compare( lhs, rhs ) != 0; }

    /** \relates IdStr Less */
    inline bool operator<( const IdStr & lhs, const IdStr & rhs )
    { return IdStr::compare( lhs, rhs ) < 0; }
    /** \overload */
    inline bool operator<( const IdStr & lhs, const char * rhs )
    { return IdStr::compare( lhs, rhs ) < 0; }
    /** \overload */
    inline bool operator<( const IdStr & lhs, const std::string & rhs )
    { return IdStr::compare( lhs, rhs ) < 0; }
    /** \overload */
    inline bool operator<( const char * lhs, const IdStr & rhs )
    { return IdStr::compare( lhs, rhs ) < 0; }
    /** \overload */
    inline bool operator<( const std::string & lhs, const IdStr & rhs )
    { return IdStr::compare( lhs, rhs ) < 0; }

    /** \relates IdStr LessEqual*/
    inline bool operator<=( const IdStr & lhs, const IdStr & rhs )
    { return IdStr::compare( lhs, rhs ) <= 0; }
    /** \overload */
    inline bool operator<=( const IdStr & lhs, const char * rhs )
    { return IdStr::compare( lhs, rhs ) <= 0; }
    /** \overload */
    inline bool operator<=( const IdStr & lhs, const std::string & rhs )
    { return IdStr::compare( lhs, rhs ) <= 0; }
    /** \overload */
    inline bool operator<=( const char * lhs, const IdStr & rhs )
    { return IdStr::compare( lhs, rhs ) <= 0; }
    /** \overload */
    inline bool operator<=( const std::string & lhs, const IdStr & rhs )
    { return IdStr::compare( lhs, rhs ) <= 0; }

     /** \relates IdStr Greater */
    inline bool operator>( const IdStr & lhs, const IdStr & rhs )
    { return IdStr::compare( lhs, rhs ) > 0; }
    /** \overload */
    inline bool operator>( const IdStr & lhs, const char * rhs )
    { return IdStr::compare( lhs, rhs ) > 0; }
    /** \overload */
    inline bool operator>( const IdStr & lhs, const std::string & rhs )
    { return IdStr::compare( lhs, rhs ) > 0; }
    /** \overload */
    inline bool operator>( const char * lhs, const IdStr & rhs )
    { return IdStr::compare( lhs, rhs ) > 0; }
    /** \overload */
    inline bool operator>( const std::string & lhs, const IdStr & rhs )
    { return IdStr::compare( lhs, rhs ) > 0; }

    /** \relates IdStr GreaterEqual*/
    inline bool operator>=( const IdStr & lhs, const IdStr & rhs )
    { return IdStr::compare( lhs, rhs ) >= 0; }
    /** \overload */
    inline bool operator>=( const IdStr & lhs, const char * rhs )
    { return IdStr::compare( lhs, rhs ) >= 0; }
    /** \overload */
    inline bool operator>=( const IdStr & lhs, const std::string & rhs )
    { return IdStr::compare( lhs, rhs ) >= 0; }
    /** \overload */
    inline bool operator>=( const char * lhs, const IdStr & rhs )
    { return IdStr::compare( lhs, rhs ) >= 0; }
    /** \overload */
    inline bool operator>=( const std::string & lhs, const IdStr & rhs )
    { return IdStr::compare( lhs, rhs ) >= 0; }

   /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SAT_IDSTR_H
