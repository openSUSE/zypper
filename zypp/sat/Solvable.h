/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/Solvable.h
 *
*/
#ifndef ZYPP_SAT_SOLVABLE_H
#define ZYPP_SAT_SOLVABLE_H

#include <iosfwd>

#include "zypp/base/SafeBool.h"
#include "zypp/base/Iterator.h"

///////////////////////////////////////////////////////////////////
extern "C"
{
struct _Solvable;
}
///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////

    class Repo;

    class PoolId : private base::SafeBool<PoolId>
    {
      public:
        PoolId() : _id( 0 ) {}
        PoolId( int id_r ) : _id( id_r ) {}
        static const PoolId noid;
        /** Evaluate \ref PoolId in a boolean context (\c != \c 0). */
        using base::SafeBool<PoolId>::operator bool_type;
        int get() const { return _id; }
      private:
        friend base::SafeBool<PoolId>::operator bool_type() const;
        bool boolTest() const { return _id; }
      private:
        int _id;
    };

    /** \relates PoolId Stream output */
    std::ostream & operator<<( std::ostream & str, const PoolId & obj );

    /** \relates PoolId */
    inline bool operator==( const PoolId & lhs, const PoolId & rhs )
    { return lhs.get() == rhs.get(); }

    /** \relates PoolId */
    inline bool operator!=( const PoolId & lhs, const PoolId & rhs )
    { return lhs.get() != rhs.get(); }

    typedef PoolId NameId;
    typedef PoolId EvrId;
    typedef PoolId ArchId;
    typedef PoolId VendorId;

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Solvable
    //
    /** */
    class Solvable : private base::SafeBool<Solvable>
    {
      public:
        /** Ctor defaults to \ref nosolvable.*/
        Solvable( ::_Solvable * solvable_r = NULL )
        : _solvable( solvable_r )
        {}

      public:
        /** Represents no \ref Solvable. */
        static const Solvable nosolvable;

        /** Evaluate \ref Solvable in a boolean context (\c != \c nosolvable). */
        using base::SafeBool<Solvable>::operator bool_type;

      public:
        NameId   name() const;
        EvrId    evr() const;
        ArchId   arch() const;
        VendorId vendor() const;

        Repo repo() const;

      public:
        const char * string( const PoolId & id_r ) const;

        const char * nameStr() const { return string( name() ); }
        const char * evrStr() const { return string( evr() ); }
        const char * archStr() const { return string( arch() ); }
        const char * vendorStr() const { return string( vendor() ); }

      public:
        /** Expert backdoor. */
        ::_Solvable * get() const { return _solvable; }
      private:
        friend base::SafeBool<Solvable>::operator bool_type() const;
        bool boolTest() const { return _solvable; }
      private:
        ::_Solvable * _solvable;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates Solvable Stream output */
    std::ostream & operator<<( std::ostream & str, const Solvable & obj );

    /** \relates Solvable */
    inline bool operator==( const Solvable & lhs, const Solvable & rhs )
    { return lhs.get() == rhs.get(); }

    /** \relates Solvable */
    inline bool operator!=( const Solvable & lhs, const Solvable & rhs )
    { return lhs.get() != rhs.get(); }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : SolvableIterator
    //
    /** */
    class SolvableIterator : public boost::iterator_adaptor<
                                      SolvableIterator                  // Derived
                                      , ::_Solvable *                   // Base
                                      , Solvable                        // Value
                                      , boost::forward_traversal_tag    // CategoryOrTraversal
                                      , Solvable                        // Reference
                                    >
    {
      public:
        SolvableIterator()
        : SolvableIterator::iterator_adaptor_( 0 )
        {}

        explicit SolvableIterator( ::_Solvable * p )
        : SolvableIterator::iterator_adaptor_( p )
        {}

      private:
        friend class boost::iterator_core_access;

        void increment();

        Solvable dereference() const
        { return base(); }
    };
    ///////////////////////////////////////////////////////////////////

   /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SAT_SOLVABLE_H
