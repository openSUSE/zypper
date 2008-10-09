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

    ///////////////////////////////////////////////////////////////////
    //
    // class DeltaCandidates
    //
    ///////////////////////////////////////////////////////////////////

    DeltaCandidates::DeltaCandidates(const std::list<Repository> & repos,
                                     const std::string & pkgname)
    : _pimpl( new Impl(repos, pkgname) )
    {}

    DeltaCandidates::~DeltaCandidates()
    {}

    std::list<DeltaRpm> DeltaCandidates::deltaRpms(const Package::constPtr & package) const
    {
      std::list<DeltaRpm> candidates;

      // query all repos
      for ( std::list<Repository>::const_iterator it = _pimpl->repos.begin();
            it != _pimpl->repos.end(); ++it )
      {
        DBG << "package: " << package << endl;
        {
          ::Dataiterator di;
          ::dataiterator_init(&di
            , it->get()                                              // in this repo
            , REPOENTRY_META                                         // in metadata
            , REPOSITORY_DELTAINFO, 0, 0 );

          while (::dataiterator_step(&di))
          {
              ::dataiterator_setpos( &di );
              ::Dataiterator di2;
              ::dataiterator_init(&di2
                  , it->get()                                       // in this repo
                  , REPOENTRY_POS                                   // in metadata
                  , DELTA_PACKAGE_NAME
                  , _pimpl->pkgname.empty() ? 0 : _pimpl->pkgname.c_str()  // of this value
                  , SEARCH_STRING);
              while (::dataiterator_step(&di2))
              {
                ::dataiterator_setpos( &di2 );
                DeltaRpm delta(*it, REPOENTRY_POS);
                DBG << "checking delta: " << delta << endl;
                if ( ! package
                       || (    package->name()    == delta.name()
                       && package->edition() == delta.edition()
                       && package->arch()    == delta.arch() ) )
                {
                  DBG << "got delta candidate" << endl;
                  candidates.push_back( delta );
                }
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
