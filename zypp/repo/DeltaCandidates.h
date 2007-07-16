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
      /** 
       * \short Creates a candidate calculator
       * \param repos Set of repositories providing patch and delta packages
       * \param installed_callback Will be used to ask if a package is installed or not
       */
      DeltaCandidates( const std::list<Repository> & repos );
      /** Dtor */
      ~DeltaCandidates();

      
      std::list<packagedelta::PatchRpm> patchRpms(const Package::constPtr & package) const;
      std::list<packagedelta::DeltaRpm> deltaRpms(const Package::constPtr & package) const;
      
    public:

    private:
      /** Pointer to implementation */
      RWCOW_pointer<Impl> _pimpl;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates DeltaCandidates Stream output */
    std::ostream & operator<<( std::ostream & str, const DeltaCandidates & obj );

    /////////////////////////////////////////////////////////////////
  } // namespace repo
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_REPO_DELTACANDIDATES_H
