/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/Capability.h
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
    //	CLASS NAME : Capability
    //
    /** A sat capability.
    */
    class Capability: protected detail::PoolMember,
                      private base::SafeBool<Capability>
    {
      public:
        /** Default ctor, no capability. */
        Capability() : _id( Null.id() ) {}
        /** Ctor from id. */
        explicit Capability( detail::IdType id_r ) : _id( id_r ) {}
        /** Ctor from string.
         * If no \c kind_r is provided, the \ref Capability refers to a \c package.
         */
        explicit Capability( const char * str_r, const KindId & kind_r = KindId::Null );
        /** Ctor from string.
         * If no \c kind_r is provided, the \ref Capability refers to a \c package.
         */
        explicit Capability( const std::string & str_r, const KindId & kind_r = KindId::Null );
        /** Ctor from <tt>name op edition</tt>.
         * If no \c kind_r is provided, the \ref Capability refers to a \c package.
         */
        Capability( const std::string & name_r, Rel op_r, const Edition & ed_r, const KindId & kind_r = KindId::Null );

        /** Evaluate in a boolean context (\c != \c Null). */
        using base::SafeBool<Capability>::operator bool_type;
      public:
        /** No or Null \ref Capability. */
        static const Capability Null;

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
        friend base::SafeBool<Capability>::operator bool_type() const;
        bool boolTest() const { return _id; }
      private:
        detail::IdType _id;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates Capability Stream output */
    std::ostream & operator<<( std::ostream & str, const Capability & obj );

    /** \relates Capability */
    inline bool operator==( const Capability & lhs, const Capability & rhs )
    { return lhs.id() == rhs.id(); }

    /** \relates Capability */
    inline bool operator!=( const Capability & lhs, const Capability & rhs )
    { return lhs.id() != rhs.id(); }


    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SAT_IDREL_H
