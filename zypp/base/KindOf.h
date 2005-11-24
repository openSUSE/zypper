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
  namespace base
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
     * like to be exensible at runtime.
     *
     * KindOf stores a \b lowercased version of a string and uses this as
     * identification.
     *
     * \todo How to make doxygen show the related ==/!= operator
     * for KindOf vs std::string comarison?
     * \todo Unify strings and associate numerical value for more
     * efficient comparison and use in \c switch.
     * \todo Make lowercased/uppercased/... an option. First of all
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
        : _value( string::toLower(value_r) )
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

    /** \Relates KindOf stream output*/
    template<class _Tp>
      inline std::ostream & operator<<( std::ostream & str, const KindOf<_Tp> & obj )
      { return str << obj.asString(); }

    ///////////////////////////////////////////////////////////////////

    /** \Relates KindOf */
    template<class _Tp>
      inline bool operator==( const KindOf<_Tp> & lhs, const KindOf<_Tp> & rhs )
      { return lhs.asString() == rhs.asString(); }

    /** \Relates KindOf */
    template<class _Tp>
      inline bool operator==( const KindOf<_Tp> & lhs, const std::string & rhs )
      { return lhs.asString() == string::toLower(rhs); }

    /** \Relates KindOf */
    template<class _Tp>
      inline bool operator==( const std::string & lhs, const KindOf<_Tp> & rhs )
      { return string::toLower(lhs) == rhs.asString(); }


    /** \Relates KindOf */
    template<class _Tp>
      inline bool operator!=( const KindOf<_Tp> & lhs, const KindOf<_Tp> & rhs )
      { return !( lhs == rhs ); }

    /** \Relates KindOf */
    template<class _Tp>
      inline bool operator!=( const KindOf<_Tp> & lhs, const std::string & rhs )
      { return !( lhs == rhs ); }

    /** \Relates KindOf */
    template<class _Tp>
      inline bool operator!=( const std::string & lhs, const KindOf<_Tp> & rhs )
      { return !( lhs == rhs ); }


    /** \Relates KindOf For use in std::container. */
    template<class _Tp>
      inline bool operator<( const KindOf<_Tp> & lhs, const KindOf<_Tp> & rhs )
      { return lhs.asString() < rhs.asString(); }

    /////////////////////////////////////////////////////////////////
  } // namespace base
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_KINDOF_H
