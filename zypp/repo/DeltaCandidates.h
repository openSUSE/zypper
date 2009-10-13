/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_REPO_DELTACANDIDATES_H
#define ZYPP_REPO_DELTACANDIDATES_H

#include <iosfwd>
#include <list>

#include "zypp/base/PtrTypes.h"
#include "zypp/base/Function.h"
#include "zypp/repo/PackageDelta.h"
#include "zypp/Repository.h"
#include "zypp/Package.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace repo
  { /////////////////////////////////////////////////////////////////

    /**
     * \short Candidate delta and patches for a package
     *
     * Basically a container that given N repositories,
     * gets all patches and deltas from them for a given
     * package.
     */
    class DeltaCandidates
    {
      friend std::ostream & operator<<( std::ostream & str, const DeltaCandidates & obj );

    public:
      /** Implementation  */
      class Impl;

    public:
      DeltaCandidates();
      /**
       * \short Creates a candidate calculator
       * \param repos Set of repositories providing patch and delta packages
       */
      DeltaCandidates( const std::list<Repository> & repos, const std::string & pkgname = "" );
      /** Dtor */
      ~DeltaCandidates();

      std::list<packagedelta::DeltaRpm> deltaRpms(const Package::constPtr & package) const;

    private:
      /** Pointer to implementation */
      RWCOW_pointer<Impl> _pimpl;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates DeltaCandidates Stream output */
    std::ostream & operator<<( std::ostream & str, const DeltaCandidates & obj );

    ///////////////////////////////////////////////////////////////////

    /** \relates DeltaCandidates Convenient construction.
     * \todo templated ctor
    */
    template<class RepoIter>
    inline DeltaCandidates makeDeltaCandidates( RepoIter begin_r, RepoIter end_r )
    { return DeltaCandidates( std::list<Repository>( begin_r, end_r ) ); }

    /** \relates DeltaCandidates Convenient construction.
     * \todo templated ctor
     */
    template<class RepoContainer>
    inline DeltaCandidates makeDeltaCandidates( const RepoContainer & cont_r )
    { return makeDeltaCandidates( cont_r.begin(), cont_r.end() ); }


    /////////////////////////////////////////////////////////////////
  } // namespace repo
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_REPO_DELTACANDIDATES_H
