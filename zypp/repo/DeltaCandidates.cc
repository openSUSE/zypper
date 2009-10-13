/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
extern "C"
{
#include <satsolver/knownid.h>
}

#include <iostream>
#include "zypp/base/Logger.h"
#include "zypp/Repository.h"
#include "zypp/repo/DeltaCandidates.h"
#include "zypp/sat/Pool.h"


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
        Impl()
        {}

        Impl( const std::list<Repository> & repos, const std::string & pkgname = "" )
        : repos(repos), pkgname(pkgname)
        {}

        std::list<Repository> repos;
        std::string pkgname;

      private:
        friend Impl * rwcowClone<Impl>( const Impl * rhs );
        /** clone for RWCOW_pointer */
        Impl * clone() const
        { return new Impl( *this ); }
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates DeltaCandidates::Impl Stream output */
    inline std::ostream & operator<<( std::ostream & str, const DeltaCandidates::Impl & obj )
    {
      return str << "DeltaCandidates::Impl";
    }

    ///////////////////////////////////////////////////////////////////
    //
    // class DeltaCandidates
    //
    ///////////////////////////////////////////////////////////////////

    DeltaCandidates::DeltaCandidates()
    : _pimpl( new Impl )
    {}


    DeltaCandidates::DeltaCandidates(const std::list<Repository> & repos,
                                     const std::string & pkgname)
    : _pimpl( new Impl(repos, pkgname) )
    {}

    DeltaCandidates::~DeltaCandidates()
    {}

    std::list<DeltaRpm> DeltaCandidates::deltaRpms(const Package::constPtr & package) const
    {
      std::list<DeltaRpm> candidates;

      DBG << "package: " << package << endl;
      for_( rit, _pimpl->repos.begin(), _pimpl->repos.end() )
      {
        sat::LookupRepoAttr q( sat::SolvAttr::repositoryDeltaInfo, *rit );
        for_( it, q.begin(), q.end() )
        {
          if ( _pimpl->pkgname.empty()
               || it.subFind( sat::SolvAttr(DELTA_PACKAGE_NAME) ).asString() == _pimpl->pkgname )
          {
            DeltaRpm delta( it );
            //DBG << "checking delta: " << delta << endl;
            if ( ! package
                   || (    package->name()    == delta.name()
                        && package->edition() == delta.edition()
                        && package->arch()    == delta.arch() ) )
            {
              DBG << "got delta candidate: " << delta << endl;
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
