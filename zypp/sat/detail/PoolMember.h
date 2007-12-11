/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/detail/PoolMember.h
 *
*/
#ifndef ZYPP_SAT_DETAIL_POOLMEMBER_H
#define ZYPP_SAT_DETAIL_POOLMEMBER_H

#include "zypp/base/Iterator.h"

extern "C"
{
struct _Solvable;
struct _Repo;
struct _Pool;
}

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////

    class IdStr;
    class Capability;
    class Capabilities;
    class Solvable;
    class Repo;
    class Pool;

    ///////////////////////////////////////////////////////////////////
    namespace detail
    { /////////////////////////////////////////////////////////////////

      class PoolImpl;

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : PoolMember
      //
      /** Backlink to the associated \ref PoolImpl.
       * Simple as we currently use one global PoolImpl. If we change our
       * minds this is where we'd store and do the \c Id to \ref PoolImpl
       * mapping.
       */
      struct PoolMember
      {
        static PoolImpl & myPool();
      };
      ///////////////////////////////////////////////////////////////////

      /////////////////////////////////////////////////////////////////
    } // namespace detail
    ///////////////////////////////////////////////////////////////////


    ///////////////////////////////////////////////////////////////////
    namespace detail
    { /////////////////////////////////////////////////////////////////

      /** Generic Id type. */
      typedef int IdType;
      static const IdType noId( 0 );

      /** Id type to connect \ref Solvable and sat-solvable.
       * Indext into solvable array.
      */
      typedef unsigned SolvableIdType;
      /** Id to denote \ref Solvable::nosolvable. */
      static const SolvableIdType noSolvableId( 0 );

      /** Id type to connect \ref Repo and sat-repo. */
      typedef ::_Repo * RepoIdType;
      /** Id to denote \ref Repo::nosolvable. */
      static const RepoIdType noRepoId( 0 );

      /////////////////////////////////////////////////////////////////
    } // namespace detail
    ///////////////////////////////////////////////////////////////////


    ///////////////////////////////////////////////////////////////////
    namespace detail
    { /////////////////////////////////////////////////////////////////

      class SolvableIterator;
      class RepoIterator;
      class ByRepo;

      /////////////////////////////////////////////////////////////////
    } // namespace detail
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    typedef IdStr KindId;
    typedef IdStr NameId;
    typedef IdStr EvrId;
    typedef IdStr ArchId;
    typedef IdStr VendorId;
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SAT_DETAIL_POOLMEMBER_H
