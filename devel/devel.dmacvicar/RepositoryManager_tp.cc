
#include <sys/time.h>

#include <iostream>
#include <fstream>

#include <zypp/base/Logger.h>
#include <zypp/ZYpp.h>
#include <zypp/ZYppFactory.h>

#include "zypp/Product.h"
#include "zypp/detail/PackageImplIf.h"
#include "zypp/Package.h"
#include "zypp2/RepositoryFactory.h"
#include "zypp2/RepoInfo.h"

#include "zypp2/repo/cached/RepoImpl.h"
#include "zypp/data/ResolvableData.h"

#include "zypp2/RepoManager.h"
#include "zypp2/RepoInfo.h"


using namespace std;
using namespace zypp;
using namespace zypp::source;


int main(int argc, char **argv)
{
    try
    {
      ZYpp::Ptr z = getZYpp();
    
      Pathname dbpath = Pathname(getenv("PWD"));
      
      RepoManager manager;
      RepoInfoList repos = manager.knownRepositories();

      for ( RepoInfoList::const_iterator it = repos.begin(); it != repos.end(); ++it )
      {
        cout << *it << endl;
      }
    }
    catch ( const Exception &e )
    {
      ZYPP_CAUGHT(e);
      cout << e.msg() << endl;
    }
    
    return 0;
}
