
#include <sys/time.h>

#include <iostream>
#include <fstream>

#include <zypp/base/Logger.h>
#include <zypp/base/Measure.h>
#include <zypp/ZYpp.h>
#include <zypp/ZYppFactory.h>

#include "zypp/Product.h"
#include "zypp/detail/PackageImplIf.h"
#include "zypp/Package.h"
#include "zypp/RepoInfo.h"

#include "zypp/repo/cached/RepoImpl.h"
#include "zypp/data/ResolvableData.h"

#include "zypp/RepoManager.h"
#include "zypp/RepoInfo.h"


using namespace std;
using namespace zypp;
using namespace zypp::repo;


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
        Repository repo = manager.createFromCache(*it);
        //z->addResolvables(repo.resolvables());
      }

      ResPool pool(z->pool());
      cout << pool.size() << " resolvables" << endl;
      debug::Measure m("group call");
      for ( ResPool::const_iterator it = pool.begin();
            it != pool.end();
            ++it )
      {
        ResObject::constPtr res = (*it).resolvable();
        if ( isKind<Package>(res) )
        {
          Package::constPtr p = asKind<Package>(res);
          //cout << p->group() << std::endl;
        }
      }
      m.elapsed();
    }
    catch ( const Exception &e )
    {
      ZYPP_CAUGHT(e);
      cout << e.msg() << endl;
    }
    
    return 0;
}
