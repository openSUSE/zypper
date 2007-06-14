#include <cassert>
#include <iostream>

#include "zypp/Repository.h"
#include "zypp/repo/RepositoryImpl.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  const Repository Repository::noRepository;

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Repository::Repository
  //	METHOD TYPE : Ctor
  //
  Repository::Repository()
  : _pimpl( Impl::nullimpl() )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Repository::Repository
  //	METHOD TYPE : Ctor
  //
  Repository::Repository( const Impl_Ptr & impl_r )
  : _pimpl( impl_r )
  {
    assert( impl_r );
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	Forward to RepositoryImpl:
  //
  ///////////////////////////////////////////////////////////////////

  Repository::NumericId Repository::numericId() const
  { return _pimpl->numericId(); }

  const ResStore & Repository::resolvables()
  {
    return _pimpl->resolvables();
  }
  
  const RepoInfo Repository::info() const
  {
    return _pimpl->info();
  }
  
  const std::list<packagedelta::PatchRpm> &
  Repository::patchRpms() const
  {
    return _pimpl->patchRpms();
  }
    
  const std::list<packagedelta::DeltaRpm> &
  Repository::deltaRpms() const
  {
    return _pimpl->deltaRpms();
  }
  
  
}

