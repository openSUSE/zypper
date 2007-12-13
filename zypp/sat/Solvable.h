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

#include "zypp/sat/detail/PoolMember.h"
#include "zypp/sat/Capabilities.h"
#include "zypp/sat/Capability.h"
#include "zypp/sat/IdStr.h"

#include "zypp/Dep.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Solvable
    //
    /** */
    class Solvable : protected detail::PoolMember,
                     private base::SafeBool<Solvable>
    {
      public:
        /** Default ctor creates \ref nosolvable.*/
        Solvable()
        : _id( detail::noSolvableId ) {}

        /** \ref PoolImpl ctor. */
        explicit Solvable( detail::SolvableIdType id_r )
        : _id( id_r ) {}

      public:
        /** Represents no \ref Solvable. */
        static const Solvable nosolvable;

        /** Evaluate \ref Solvable in a boolean context (\c != \c nosolvable). */
        using base::SafeBool<Solvable>::operator bool_type;

        /** Return whether this \ref Solvable belongs to the system repo. */
        bool isSystem() const;

        /** The \ref Repository this \ref Solvable belongs to. */
        Repo repo() const;

      public:
        NameId   name()   const;
        EvrId    evr()    const;
        ArchId   arch()   const;
        VendorId vendor() const;

      public:

        /** \name Access to the \ref Solvable dependencies.
         *
         * \note Prerequires are a subset of requires.
         */
        //@{
        Capabilities operator[]( Dep which_r ) const;

        Capabilities provides()    const;
        Capabilities requires()    const;
        Capabilities conflicts()   const;
        Capabilities obsoletes()   const;
        Capabilities recommends()  const;
        Capabilities suggests()    const;
        Capabilities freshens()    const;
        Capabilities enhances()    const;
        Capabilities supplements() const;
        Capabilities prerequires() const;
        //@}


      public:
        /** Return next Solvable in \ref Pool (or \ref nosolvable). */
        Solvable nextInPool() const;
        /** Return next Solvable in \ref Repo (or \ref nosolvable). */
        Solvable nextInRepo() const;

      public:
        /** Expert backdoor. */
        ::_Solvable * get() const;
        /** Expert backdoor. */
        detail::SolvableIdType id() const { return _id; }
      private:
        friend base::SafeBool<Solvable>::operator bool_type() const;
        bool boolTest() const { return get(); }
      private:
        detail::SolvableIdType _id;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates Solvable Stream output */
    std::ostream & operator<<( std::ostream & str, const Solvable & obj );

    /** \relates Solvable More verbose stream output including dependencies */
    std::ostream & dumpOn( std::ostream & str, const Solvable & obj );

    /** \relates Solvable */
    inline bool operator==( const Solvable & lhs, const Solvable & rhs )
    { return lhs.get() == rhs.get(); }

    /** \relates Solvable */
    inline bool operator!=( const Solvable & lhs, const Solvable & rhs )
    { return lhs.get() != rhs.get(); }

    /** \relates Solvable */
    inline bool operator<( const Solvable & lhs, const Solvable & rhs )
    { return lhs.get() < rhs.get(); }

    ///////////////////////////////////////////////////////////////////
    namespace detail
    { /////////////////////////////////////////////////////////////////
      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : SolvableIterator
      //
      /** */
      class SolvableIterator : public boost::iterator_adaptor<
          SolvableIterator                   // Derived
          , ::_Solvable*                     // Base
          , Solvable                         // Value
          , boost::single_pass_traversal_tag // CategoryOrTraversal
          , Solvable                         // Reference
          >
      {
        public:
          SolvableIterator()
          : SolvableIterator::iterator_adaptor_( 0 )
          {}

          explicit SolvableIterator( const Solvable & val_r )
          : SolvableIterator::iterator_adaptor_( 0 )
          { assignVal( val_r ); }

          explicit SolvableIterator( SolvableIdType id_r )
          : SolvableIterator::iterator_adaptor_( 0 )
          { assignVal( Solvable( id_r ) ); }

        private:
          friend class boost::iterator_core_access;

          void increment()
          { assignVal( _val.nextInPool() ); }

          Solvable dereference() const
          { return _val; }

          void assignVal( const Solvable & val_r )
          { _val = val_r; base_reference() = _val.get(); }

          Solvable _val;
      };
      ///////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////
    } // namespace detail
    ///////////////////////////////////////////////////////////////////

   /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SAT_SOLVABLE_H
