/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/LookupAttr.h
 *
*/
#ifndef ZYPP_SAT_LOOKUPATTR_H
#define ZYPP_SAT_LOOKUPATTR_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"
#include "zypp/sat/Pool.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : LookupAttr
    //
    /** Lightweight attribute lookup.
    */
    class LookupAttr
    {
      public:
        /** Default ctor. */
        LookupAttr()
        {}
        /** Lookup \ref SolvAttr in \ref Pool (all repositories). */
        LookupAttr( SolvAttr attr_r )
        : _attr( attr_r )
        {}
        /** Lookup \ref SolvAttr in one \ref Repository. */
        LookupAttr( SolvAttr attr_r, Repository repo_r )
        : _attr( attr_r ), _repo( repo_r )
        {}
        /** Lookup \ref SolvAttr in one \ref Solvable. */
        LookupAttr( SolvAttr attr_r, Solvable solv_r )
        : _attr( attr_r ), _solv( solv_r )
        {}

      public:
        /** \name Search result. */
        //@{
        /** Result iterator. */
        class iterator;

        /** Iterator to the begin of query results. */
        iterator begin() const;

        /** Iterator behind the end of query results. */
        iterator end() const;

        //@}
      public:
        /** \name What to search. */
        //@{

        /** The \ref SolvAttr to search. */
        SolvAttr attr() const
        { return _attr; }

        /** Set the \ref SolvAttr to search. */
        void setAttr( SolvAttr attr_r )
        { _attr = attr_r; }

        //@}
      public:
        /** \name Where to search. */
        //@{
        /** Wheter to search in \ref Pool. */
        bool pool() const
        { return ! (_repo || _solv); }

        /** Set search in \ref Pool (all repositories). */
        void setPool()
        {
          _repo = Repository::noRepository;
          _solv = Solvable::noSolvable;
        }

        /** Wheter to search in one \ref Repository. */
        Repository repo() const
        { return _repo; }

        /** Set search in one \ref Repository. */
        void setRepo( Repository repo_r )
        {
          _repo = repo_r;
          _solv = Solvable::noSolvable;
        }

        /** Wheter to search in one \ref Solvable. */
        Solvable solvable() const
        { return _solv; }

        /** Set search in one \ref Solvable. */
        void setSolvable( Solvable solv_r )
         {
          _repo = Repository::noRepository;
          _solv = solv_r;
        }

        //@}
      private:
        SolvAttr   _attr;
        Repository _repo;
        Solvable   _solv;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates Arch stream output. */
    std::ostream & operator<<( std::ostream & str, const LookupAttr & obj );

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : LookupAttr::iterator
    //
    /** Result iterator.
    */
    struct LookupAttr::iterator
    {
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SAT_LOOKUPATTR_H
