
#ifndef ZYPP_REPOSITORY_FACTORY_H
#define ZYPP_REPOSITORY_FACTORY_H

#include "zypp2/Repository.h"
#include "zypp2/RepoInfo.h"

namespace zypp
{
  class RepositoryFactory
  {
    friend std::ostream & operator<<( std::ostream & str, const RepositoryFactory & obj );

    public:
    /** Default ctor */
    RepositoryFactory();
    /** Dtor */
    ~RepositoryFactory();

  public:
    /** Construct source.
     * \throw EXCEPTION on fail
     */
    Repository createFrom( const RepoInfo & context );
  };
}

#endif


