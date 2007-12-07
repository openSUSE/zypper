/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/IdRel.h
 *
*/
#ifndef ZYPP_SAT_IDREL_H
#define ZYPP_SAT_IDREL_H

#include <iosfwd>
#include <string>

#include "zypp/base/SafeBool.h"

#include "zypp/sat/detail/PoolMember.h"
#include "zypp/sat/IdStr.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class Rel;
  class Edition;

  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : IdRel
    //
    /** Access to the sat-pools string space.
     * Construction from string will place a copy of the string in the
     * string space, if it is not already present.
    */
    class IdRel: protected detail::PoolMember,
                 private base::SafeBool<IdRel>
    {
      public:
        /** Default ctor, no relation. */
        IdRel() : _id( Null.id() ) {}
        /** Ctor from id. */
        explicit IdRel( detail::IdType id_r ) : _id( id_r ) {}
        /** Ctor from string.
         * If \c kind_r is provided, \c name is prefixed.
         */
        explicit IdRel( const char * str_r, const KindId & kind_r = KindId::Null );
        /** Ctor from string.
         * If \c kind_r is provided, \c name is prefixed.
         */
        explicit IdRel( const std::string & str_r, const KindId & kind_r = KindId::Null );
        /** Ctor from <tt>name op edition</tt>.
         * If \c kind_r is provided, \c name is prefixed.
         */
        IdRel( const std::string & name_r, Rel op_r, const Edition & ed_r, const KindId & kind_r = KindId::Null );

        /** Evaluate in a boolean context (\c != \c Null). */
        using base::SafeBool<IdRel>::operator bool_type;
      public:
        /** No or Null relation. */
        static const IdRel Null;
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
        friend base::SafeBool<IdRel>::operator bool_type() const;
        bool boolTest() const { return _id; }
      private:
        detail::IdType _id;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates IdRel Stream output */
    std::ostream & operator<<( std::ostream & str, const IdRel & obj );

    /** \relates IdRel */
    inline bool operator==( const IdRel & lhs, const IdRel & rhs )
    { return lhs.id() == rhs.id(); }

    /** \relates IdRel */
    inline bool operator!=( const IdRel & lhs, const IdRel & rhs )
    { return lhs.id() != rhs.id(); }


    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SAT_IDREL_H
