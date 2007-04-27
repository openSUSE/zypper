
#ifndef ZYPP_NEW_REPOMANAGER_H
#define ZYPP_NEW_REPOMANAGER_H

#include <list>
#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/Pathname.h"

#include "zypp2/RepositoryInfo.h"

namespace zypp
{
  
  class RepositoryManager : public base::ReferenceCounted, private base::NonCopyable
  {
    friend std::ostream & operator<<( std::ostream & str, const RepositoryManager & obj );
  public:
    RepositoryManager();
    std::list<RepositoryInfo> knownRepositories();
  };
  
  
}

#endif

