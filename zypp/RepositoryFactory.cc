
#include "zypp/RepositoryFactory.h"

namespace zypp {

RepositoryFactory::RepositoryFactory()
{
}

RepositoryFactory::~RepositoryFactory()
{

}


Repository RepositoryFactory::createFrom( const RepoInfo & context )
{
  return Repository::noRepository;
}

} // ns zypp


