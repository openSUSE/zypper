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
#include "zypp/Repository.h"
#include "zypp/repo/DeltaCandidates.h"

extern "C"
{
#include <satsolver/repo.h>
}

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

      Impl( const std::list<Repository> & repos, const std::string & pkgname = "" )
        : repos(repos), pkgname(pkgname)
      {

      }

      friend Impl * rwcowClone<Impl>( const Impl * rhs );
      /** clone for RWCOW_pointer */
      Impl * clone() const
      { return new Impl( *this ); }

      std::list<Repository> repos;
      std::string pkgname;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates DeltaCandidates::Impl Stream output */
    inline std::ostream & operator<<( std::ostream & str, const DeltaCandidates::Impl & obj )
    {
      return str << "DeltaCandidates::Impl";
    }

    DeltaCandidates::DeltaCandidates(const std::list<Repository> & repos,
                                     const std::string & pkgname)
    : _pimpl( new Impl(repos, pkgname) )
    {}

    DeltaCandidates::~DeltaCandidates()
    {}

    std::list<PatchRpm> DeltaCandidates::patchRpms(const Package::constPtr & package ) const
    {
      std::list<PatchRpm> candidates;

      // query all repos
      for ( std::list<Repository>::const_iterator it = _pimpl->repos.begin();
            it != _pimpl->repos.end();
            ++it )
      {
        // all delta in repo
       #warning patchRpms are not implemented
	std::list<PatchRpm> candidates_in_repo; // = (*it).patchRpms();
        for ( std::list<PatchRpm>::const_iterator dit = candidates_in_repo.begin();
              dit != candidates_in_repo.end();
              ++dit )
        {
          if ( ! package
               || (    package->name()    == dit->name()
                    && package->edition() == dit->edition()
                    && package->arch()    == dit->arch() ) )
          {
            candidates.push_back( *dit );
          }
        }
      }
      return candidates;
    }

    std::list<DeltaRpm> DeltaCandidates::deltaRpms(const Package::constPtr & package) const
    {
      std::list<DeltaRpm> candidates;

#warning deltaRpms are not completely implemented

      // query all repos
      for ( std::list<Repository>::const_iterator it = _pimpl->repos.begin();
            it != _pimpl->repos.end(); ++it )
      {
        for (int i = 0; i < it->get()->nextra; ++i)
        {
          ::Dataiterator di;
          ::dataiterator_init(&di
            , it->get()                                              // in this repo
            , -1 - i                                                 // in this extra
            , DELTA_PACKAGE_NAME                                     // with this attribute
            , _pimpl->pkgname.empty() ? 0 : _pimpl->pkgname.c_str()  // of this value
            , SEARCH_EXTRA | SEARCH_NO_STORAGE_SOLVABLE | SEARCH_STRING);
          while (::dataiterator_step(&di))
          {
            DeltaRpm delta(*it, di.solvid);
            std::cout << delta << endl;
            if ( ! package
                || (    package->name()    == delta.name()
                     && package->edition() == delta.edition()
                     && package->arch()    == delta.arch() ) )
            {
              candidates.push_back( delta );
            }
          }
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
