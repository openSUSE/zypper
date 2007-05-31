
#include "zypp2/repo/RepositoryImpl.h"

namespace zypp { namespace repo {

IMPL_PTR_TYPE(RepositoryImpl)

RepositoryImpl::RepositoryImpl()
{

}

RepositoryImpl::~RepositoryImpl()
{

}

RepositoryImpl::RepositoryImpl( const null & )
  : base::ProvideNumericId<RepositoryImpl,Repository::NumericId>( NULL )
{}

} } // ns

