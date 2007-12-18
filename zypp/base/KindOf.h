/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/KindOf.h
 *
*/
#ifndef ZYPP_BASE_KINDOF_H
#define ZYPP_BASE_KINDOF_H

#include <iosfwd>

#include "zypp/base/String.h"
#include "zypp/sat/IdStr.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : KindOf<_Tp>
    //
    /** Maintain string values representing different kinds of a type.
     *
     * \b Example: We have different kinds of Resolvable: Package, Patch,
     * etc. We want some thing to identify these types and have a string
     * value associated. Identification should be constructible from this
     * string. An \ref g_EnumerationClass could do this, but we would also
     * like to be extensible at runtime.
     *
     * KindOf stores a \b lowercased version of a string and uses this as
     * identification. Comparison against string values is always case insensitive.
    */
    template<class _Tp>
    class KindOf : private sat::IdStr
    {
      public:
        /** DefaultCtor: empty string */
        KindOf()
        {}
        /** Ctor from string.
         * Lowercase version of \a value_r is used as identification.
         */
        explicit
            KindOf( const std::string & value_r )
        : sat::IdStr( str::toLower(value_r) )
        {}
        /** Dtor */
        ~KindOf()
        {}
      public:
        sat::IdStr::empty;
        sat::IdStr::size;
        sat::IdStr::c_str;
        sat::IdStr::string;
        sat::IdStr::asString;
      public:
        /** Fast compare equal. */
        bool compareEQ( const KindOf & rhs ) const
        { return sat::IdStr::compareEQ( rhs ); }

        /** Compare KindOf returning <tt>-1,0,1</tt>. */
        int compare( const KindOf & rhs ) const
        { return sat::IdStr::compare( rhs ); }
        /** \overload Remember to compare case insensitive. */
        int compare( const IdStr & rhs ) const
        {
          if ( sat::IdStr::compareEQ( rhs ) )
            return 0;
          return str::compareCI( c_str(), rhs.c_str() );
        }
        /** \overload Remember to compare case insensitive.*/
        int compare( const char * rhs ) const
        { return str::compareCI( c_str(), rhs ); }
        /** \overload Remember to compare case insensitive.*/
        int compare( const std::string & rhs ) const
        { return str::compareCI( c_str(), rhs ); }
      public:
        sat::IdStr::id;
    };
    ///////////////////////////////////////////////////////////////////

    //@{
    /** \relates KindOf Stream output*/
    template<class _Tp>
      inline std::ostream & operator<<( std::ostream & str, const KindOf<_Tp> & obj )
      { return str << obj.c_str(); }
    //@}

    ///////////////////////////////////////////////////////////////////

    //@{
    /** \relates KindOf */
    template<class _Tp>
      inline bool operator==( const KindOf<_Tp> & lhs, const KindOf<_Tp> & rhs )
      { return lhs.compareEQ( rhs ); }

    /** \relates KindOf */
    template<class _Tp>
      inline bool operator==( const KindOf<_Tp> & lhs, const std::string & rhs )
      { return lhs.compare( rhs ) == 0; }

    /** \relates KindOf */
    template<class _Tp>
      inline bool operator==( const KindOf<_Tp> & lhs, const char * rhs )
      { return lhs.compare( rhs ) == 0; }

    /** \relates KindOf */
    template<class _Tp>
      inline bool operator==( const std::string & lhs, const KindOf<_Tp> & rhs )
      { return rhs.compare( lhs ) == 0; }

    /** \relates KindOf */
    template<class _Tp>
      inline bool operator==( const char * lhs, const KindOf<_Tp> & rhs )
      { return rhs.compare( lhs ) == 0; }
    //@}

    //@{
    /** \relates KindOf */
    template<class _Tp>
      inline bool operator!=( const KindOf<_Tp> & lhs, const KindOf<_Tp> & rhs )
      { return ! lhs.compareEQ( rhs ); }

    /** \relates KindOf */
    template<class _Tp>
      inline bool operator!=( const KindOf<_Tp> & lhs, const std::string & rhs )
      { return lhs.compare( rhs ) != 0; }

    /** \relates KindOf */
    template<class _Tp>
      inline bool operator!=( const KindOf<_Tp> & lhs, const char * rhs )
      { return lhs.compare( rhs ) != 0; }

    /** \relates KindOf */
    template<class _Tp>
      inline bool operator!=( const std::string & lhs, const KindOf<_Tp> & rhs )
      { return rhs.compare( lhs ) != 0; }

    /** \relates KindOf */
    template<class _Tp>
      inline bool operator!=( const char * lhs, const KindOf<_Tp> & rhs )
      { return rhs.compare( lhs ) != 0; }
    //@}

    //@{
    /** \relates KindOf std::Container order. */
    template<class _Tp>
      inline bool operator<( const KindOf<_Tp> & lhs, const KindOf<_Tp> & rhs )
      { return lhs.id() < rhs.id(); }
    //@}

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_KINDOF_H
