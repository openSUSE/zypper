#include <pdbtozypp/pdbtozypp.h>
#include <zypp/ResPool.h>
#include <zypp/RepoManager.h>
#include <zypp/TmpPath.h>
#include <zypp/base/Iterator.h>

using namespace zypp;
using namespace std;
int main(){
	
	static ZYpp::Ptr God;

	PdbToZypp pdb;
   pdb.readOut();

   ResStore store = pdb.getStore();

   /*RepoInfo repo_info;
   repo_info.setAlias("test");
   repo_info.setName("Test Repo");
   repo_info.setEnabled(true);
   repo_info.setAutorefresh(false);
   repo_info.addBaseUrl(Url("ftp://dist.suse.de/install/stable-x86/"));
	
   RepoManagerOptions opts;
   filesystem::TmpDir cachePath;
   filesystem::TmpDir rawPath;
   filesystem::TmpDir reposPath;

   opts.repoCachePath = cachePath.path();
   opts.repoRawCachePath = rawPath.path();
   opts.knownReposPath = reposPath.path();

   RepoManager repo_man(opts);

   repo_man.addRepository(repo_info);
   repo_man.refreshMetadata(repo_info);
   repo_man.buildCache(repo_info);
   Repository repo = repo_man.createFromCache(repo_info);
   ResStore store = repo.resolvables();*/

   try {
		God = zypp::getZYpp();
	}
	catch (const Exception & excpt_r ) {
		ZYPP_CAUGHT( excpt_r );
		cerr << "ZYPP no available" << endl;
		return 1;
	}

	God->addResolvables(store);
	cout << "Elements in pool: " << God->pool().size() << endl;

   for(pool::PoolTraits::const_iterator iter = God->pool().begin(); iter != God->pool().end(); iter++){

      ResObject::constPtr r = iter->resolvable();
      if(r->name() == "glibc"){
         cout << "Package found!" << endl;
         iter->status().setToBeInstalled(ResStatus::USER);
      }
   
   }

   Resolver res(God->pool());

   if(res.resolvePool() == false){
      cout << "It was not possible to solve the pool" << endl;
      list<string> problems = res.problemDescription();

      for(list<string>::iterator iter = problems.begin(); iter != problems.end(); iter++){
         cout << *iter << endl;
      }

   }else{
      cout << "The pool was solved corectly" << endl;
   }

	return 0;
}
