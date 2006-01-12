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
     * identification.
     *
     * \todo Unify strings and associate numerical value for more
     * efficient comparison and use in \c switch.
     * \todo Make lowercased/uppercased/etc an option. First of all
     * get rid of the string::toLower calls operator.
     * \todo Maybe collaboration with some sort of Registry.
    */
    template<class _Tp>
      class KindOf
      {
      public:
        /** DefaultCtor: empty string */
        KindOf()
        {}
        /** Ctor from string.
         * Lowecase version of \a value_r is used as identification.
        */
        explicit
        KindOf( const std::string & value_r )
        : _value( str::toLower(value_r) )
        {}
        /** Dtor */
        ~KindOf()
        {}
      public:
        /** Identification string. */
        const std::string & asString() const
        { return _value; }

      private:
        /** */
        std::string _value;
      };
    ///////////////////////////////////////////////////////////////////

    //@{
    /** \relates KindOf Stream output*/
    template<class _Tp>
      inline std::ostream & operator<<( std::ostream & str, const KindOf<_Tp> & obj )
      { return str << obj.asString(); }
    //@}

    ///////////////////////////////////////////////////////////////////

    //@{
    /** \relates KindOf */
    template<class _Tp>
      inline bool operator==( const KindOf<_Tp> & lhs, const KindOf<_Tp> & rhs )
      { return lhs.asString() == rhs.asString(); }

    /** \relates KindOf */
    template<class _Tp>
      inline bool operator==( const KindOf<_Tp> & lhs, const std::string & rhs )
      { return lhs.asString() == str::toLower(rhs); }

    /** \relates KindOf */
    template<class _Tp>
      inline bool operator==( const std::string & lhs, const KindOf<_Tp> & rhs )
      { return str::toLower(lhs) == rhs.asString(); }
    //@}

    //@{
    /** \relates KindOf */
    template<class _Tp>
      inline bool operator!=( const KindOf<_Tp> & lhs, const KindOf<_Tp> & rhs )
      { return !( lhs == rhs ); }

    /** \relates KindOf */
    template<class _Tp>
      inline bool operator!=( const KindOf<_Tp> & lhs, const std::string & rhs )
      { return !( lhs == rhs ); }

    /** \relates KindOf */
    template<class _Tp>
      inline bool operator!=( const std::string & lhs, const KindOf<_Tp> & rhs )
      { return !( lhs == rhs ); }
    //@}

    //@{
    /** \relates KindOf Lexicographical order. */
    template<class _Tp>
      inline bool operator<( const KindOf<_Tp> & lhs, const KindOf<_Tp> & rhs )
      { return lhs.asString() < rhs.asString(); }
    //@}

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_KINDOF_H
