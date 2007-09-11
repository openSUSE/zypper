
#include "zypp/base/Logger.h"
#include "zypp/repo/RepositoryImpl.h"

using namespace zypp;
using namespace std;

namespace zypp { namespace repo {

IMPL_PTR_TYPE(RepositoryImpl)

RepositoryImpl::RepositoryImpl( const RepoInfo &info )
  : _restore_lazy_initialized(false),
    _deltas_lazy_initialized(false),
    _info(info)
{

}

const RepoInfo & RepositoryImpl::info() const
{
  return _info;
}

RepositoryImpl::~RepositoryImpl()
{

}

RepositoryImpl::RepositoryImpl( const null & )
  : base::ProvideNumericId<RepositoryImpl,Repository::NumericId>( NULL )
{}


const ResStore & RepositoryImpl::resolvables() const
{
  if ( ! _restore_lazy_initialized )
  {
    const_cast<RepositoryImpl*>(this)->createResolvables();
    const_cast<RepositoryImpl*>(this)->_restore_lazy_initialized = true;
  }
  return _store;
}

void RepositoryImpl::createResolvables()
{
  WAR << "createResolvables() not implemented" << endl;
}

void RepositoryImpl::createPatchAndDeltas()
{
  WAR << "createPatchAndDeltas() not implemented" << endl;
}

const std::list<packagedelta::PatchRpm> &
RepositoryImpl::patchRpms() const
{
  if ( ! _deltas_lazy_initialized )
  {
    const_cast<RepositoryImpl*>(this)->createPatchAndDeltas();
    const_cast<RepositoryImpl*>(this)->_deltas_lazy_initialized = true;
  }
  return _patchRpms;
}

const std::list<packagedelta::DeltaRpm> &
RepositoryImpl::deltaRpms() const
{
  if ( ! _deltas_lazy_initialized )
  {
    const_cast<RepositoryImpl*>(this)->createPatchAndDeltas();
    const_cast<RepositoryImpl*>(this)->_deltas_lazy_initialized = true;
  }
  return _deltaRpms;
}

} } // ns

