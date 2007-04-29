
#include "repository/RepositoryImpl.h"

namespace zypp { namespace repository {

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

