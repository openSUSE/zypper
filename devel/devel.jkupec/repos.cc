#include <stdio.h>
#include <iostream>
#include <iterator>
#include <list>

#include "zypp/ZYppFactory.h"
#include "zypp/RepoInfo.h"
#include "zypp/Arch.h"
#include "zypp/Pathname.h"
#include "zypp/RepoManager.h"

using std::cout;
using std::endl;
using std::string;
using namespace zypp;


bool result_cb( const sat::Solvable & solvable )
{
  zypp::PoolItem pi( zypp::ResPool::instance().find( solvable ) );
  cout << pi.resolvable() << endl;
  // name: yast2-sound 2.16.2-9 i586
  return true;
}


static void init_pool()
{
  Pathname dir(TESTS_SRC_DIR);
  dir += "/zypp/data/PoolQuery";

  ZYpp::Ptr z = getZYpp();
  ZConfig::instance().setSystemArchitecture(Arch("i586"));

  RepoInfo i1; i1.setAlias("factory");
  sat::Pool::instance().addRepoSolv(dir / "factory.solv", i1);
  RepoInfo i2; i2.setAlias("factory-nonoss");
  sat::Pool::instance().addRepoSolv(dir / "factory-nonoss.solv", i2);
  RepoInfo i3; i3.setAlias("zypp_svn");
  sat::Pool::instance().addRepoSolv(dir / "zypp_svn.solv", i3);
  RepoInfo i4; i4.setAlias("@System");
  sat::Pool::instance().addRepoSolv(dir / "@System.solv", i4);
}


int main (int argc, const char ** argv)
{
  string _target_root = "/local/jkupec/rr";

  RepoManagerOptions repo_options(_target_root);
  // repo_options.knownReposPath = Pathname(_target_root) + repo_options.knownReposPath;

  RepoManager rm(repo_options);

  for ( RepoManager::RepoConstIterator it = rm.repoBegin();
        it != rm.repoEnd(); ++it )
  {
    cout << it->packagesPath() << endl;
  }
}
