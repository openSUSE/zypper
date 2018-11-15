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

#include "zypp/base/Hash.h"
#include "zypp/base/Iterator.h"
#include "zypp/base/String.h"
#include "zypp/base/Easy.h"

extern "C"
{
  // Those s_Type names are exposed as sat::detail::CType below!
  struct s_Dataiterator;
  struct s_Datamatcher;
  struct s_Map;
  struct s_Pool;
  struct s_Queue;
  struct s_Repo;
  struct s_Solvable;
  struct s_Solver;
  struct s_Transaction;
}

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class IdString;
  class Capability;
  class Capabilities;
  class Repository;
  class RepoInfo;

  ///////////////////////////////////////////////////////////////////
  namespace detail
  {
    class RepoIterator;
    class ByRepository;
  }

  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    namespace detail
    {
      typedef ::s_Dataiterator	CDataiterator;	///< Wrapped libsolv C data type exposed as backdoor
      typedef ::s_Datamatcher	CDatamatcher;	///< Wrapped libsolv C data type exposed as backdoor
      typedef ::s_Map		CMap;		///< Wrapped libsolv C data type exposed as backdoor
      typedef ::s_Pool		CPool;		///< Wrapped libsolv C data type exposed as backdoor
      typedef ::s_Queue		CQueue;		///< Wrapped libsolv C data type exposed as backdoor
      typedef ::s_Repo		CRepo;		///< Wrapped libsolv C data type exposed as backdoor
      typedef ::s_Solvable	CSolvable;	///< Wrapped libsolv C data type exposed as backdoor
      typedef ::s_Solver	CSolver;	///< Wrapped libsolv C data type exposed as backdoor
      typedef ::s_Transaction	CTransaction;	///< Wrapped libsolv C data type exposed as backdoor
    } // namespace detail
    ///////////////////////////////////////////////////////////////////

    class Pool;
    class Solvable;

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
      static const IdType emptyId( 1 );

      /** Internal ids satlib includes in dependencies.
       * MPL check in PoolImpl.cc
      */
      static const IdType solvablePrereqMarker( 15 );
      static const IdType solvableFileMarker  ( 16 );

      static const IdType namespaceModalias	( 18 );
      static const IdType namespaceLanguage	( 20 );
      static const IdType namespaceFilesystem	( 21 );

      /** Test for internal ids satlib includes in dependencies. */
      inline bool isDepMarkerId( IdType id_r )
      { return( id_r == solvablePrereqMarker || id_r == solvableFileMarker ); }

      /** Id type to connect \ref Solvable and sat-solvable.
       * Indext into solvable array.
      */
      typedef unsigned SolvableIdType;
      typedef SolvableIdType size_type;
      /** Id to denote \ref Solvable::noSolvable. */
      static const SolvableIdType noSolvableId( 0 );
      /** Id to denote the usually hidden \ref Solvable::systemSolvable. */
      static const SolvableIdType systemSolvableId( 1 );

      /** Id type to connect \ref Repo and sat-repo. */
      typedef CRepo * RepoIdType;
      /** Id to denote \ref Repo::noRepository. */
      static const RepoIdType noRepoId( 0 );

      /////////////////////////////////////////////////////////////////
    } // namespace detail
    ///////////////////////////////////////////////////////////////////


    ///////////////////////////////////////////////////////////////////
    namespace detail
    { /////////////////////////////////////////////////////////////////

      class SolvableIterator;

      /////////////////////////////////////////////////////////////////
    } // namespace detail
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SAT_DETAIL_POOLMEMBER_H
