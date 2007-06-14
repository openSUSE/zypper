/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include <iostream>
#include "zypp/base/Logger.h"

#include "zypp/repo/DeltaCandidates.h"

using std::endl;
using namespace zypp::packagedelta;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace repo
  { /////////////////////////////////////////////////////////////////

    /** DeltaCandidates implementation. */
    struct DeltaCandidates::Impl
    {

    public:
      
      Impl( const std::set<Repository> & repos )
        : repos(repos)
      {
      
      }

      friend Impl * rwcowClone<Impl>( const Impl * rhs );
      /** clone for RWCOW_pointer */
      Impl * clone() const
      { return new Impl( *this ); }
      
      std::set<Repository> repos;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates DeltaCandidates::Impl Stream output */
    inline std::ostream & operator<<( std::ostream & str, const DeltaCandidates::Impl & obj )
    {
      return str << "DeltaCandidates::Impl";
    }

    DeltaCandidates::DeltaCandidates(const std::set<Repository> & repos)
    : _pimpl( new Impl(repos) )
    {}

    DeltaCandidates::~DeltaCandidates()
    {}

    std::list<PatchRpm> DeltaCandidates::patchRpms(const Package::constPtr & package ) const
    {
      std::list<PatchRpm> candidates;
      
      // query all repos
      for ( std::set<Repository>::const_iterator it = _pimpl->repos.begin();
            it != _pimpl->repos.end();
            ++it )
      {
        // all delta in repo
        std::list<PatchRpm> candidates_in_repo = (*it).patchRpms();
        for ( std::list<PatchRpm>::const_iterator dit = candidates_in_repo.begin();
              dit != candidates_in_repo.end();
              ++dit )
        {
          PatchRpm delta(*dit);
          candidates.push_back(delta);
        }
      }
      return candidates;
    }
    
    std::list<DeltaRpm> DeltaCandidates::deltaRpms(const Package::constPtr & package) const
    {
      std::list<DeltaRpm> candidates;
      
      // query all repos
      for ( std::set<Repository>::const_iterator it = _pimpl->repos.begin();
            it != _pimpl->repos.end();
            ++it )
      {
        // all delta in repo
        std::list<DeltaRpm> candidates_in_repo = (*it).deltaRpms();
        for ( std::list<DeltaRpm>::const_iterator dit = candidates_in_repo.begin();
              dit != candidates_in_repo.end();
              ++dit )
        {
          DeltaRpm delta(*dit);
          candidates.push_back(delta);
        }
      }
      return candidates;
    }
    
    std::ostream & operator<<( std::ostream & str, const DeltaCandidates & obj )
    {
      return str << *obj._pimpl;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace repo
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
