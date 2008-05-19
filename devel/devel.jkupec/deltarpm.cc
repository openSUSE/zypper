#include <stdio.h>
#include <iostream>
#include <iterator>
#include <list>

extern "C"
{
#include <satsolver/repo.h>
}

#include "zypp/ZYppFactory.h"
#include "zypp/Pathname.h"

#include "zypp/RepoManager.h"
#include "zypp/repo/DeltaCandidates.h"
#include "zypp/PoolQuery.h"

using std::cout;
using std::endl;
using std::string;
using namespace zypp;


struct PrintAndCount
{
  PrintAndCount() : _count(0) {}

  bool operator()( const sat::Solvable & solvable )
  {
    zypp::PoolItem pi( zypp::ResPool::instance().find( solvable ) );
    cout << pi.resolvable() << endl;
    // name: yast2-sound 2.16.2-9 i586
    ++_count;
    return true;
  }

  unsigned _count;
};



int main (int argc, const char ** argv)
{
  Pathname rootdir(SRC_DIR "/data/deltarpm");
  RepoManagerOptions opts(rootdir);
  opts.repoRawCachePath = rootdir;
  opts.repoSolvCachePath = rootdir;
  RepoManager rm(opts);

  RepoInfo updates;
  updates.setAlias("updates");
  updates.addBaseUrl(Url(string("dir:") + rootdir.absolutename().asString() + "/updates"));

  RepoInfo updates2;
  updates2.setAlias("updates2");
  updates2.addBaseUrl(Url(string("dir:") + rootdir.absolutename().asString() + "/updates2"));

  try
  {
    rm.buildCache(updates);
    rm.buildCache(updates2);
    rm.loadFromCache(updates);
    rm.loadFromCache(updates2);
  }
  catch (const Exception & e)
  {
    cout << "Problem getting the data: " << e.msg() << endl;
  }

  sat::Pool pool(sat::Pool::instance());
  for_(repoit, pool.reposBegin(), pool.reposEnd())
  {
    Repository repo(*repoit);
    for (int i = 0; i < repo.get()->nextra; ++i)
    {
      cout << endl << "extra " << i << ":" << endl;
      ::Dataiterator di;
      ::dataiterator_init(&di, repo.get(), -1 - i, 0, 0, SEARCH_EXTRA | SEARCH_NO_STORAGE_SOLVABLE);
      while (::dataiterator_step(&di))
      {
        const char * keyname;
        keyname = ::id2str(repo.get()->pool, di.key->name);
        
        cout << keyname << ": ";

        switch (di.key->name)
        {
        case DELTA_PACKAGE_NAME:
        {
          cout << IdString(di.kv.id);
          break;
        }
        case DELTA_PACKAGE_EVR:
        {
          cout << IdString(di.kv.id);
          break;
        }
        case DELTA_PACKAGE_ARCH:
        {
          cout << IdString(di.kv.id);
          break;
        }
        case DELTA_LOCATION_DIR:
        {
          cout << IdString(di.kv.id);
          break;
        }
        case DELTA_LOCATION_NAME:
        {
          cout << IdString(di.kv.id);
          break;
        }
        case DELTA_LOCATION_EVR:
        {
          cout << IdString(di.kv.id);
          break;
        }
        case DELTA_LOCATION_SUFFIX:
        {
          cout << IdString(di.kv.id);
          break;
        }
        case DELTA_DOWNLOADSIZE:
        {
          cout << di.kv.num;
          break;
        }
        case DELTA_CHECKSUM:
        {
          cout << di.kv.str;
          break;
        }
        case DELTA_BASE_EVR:
        {
          cout << IdString(di.kv.id);
          break;
        }
        case DELTA_SEQ_NAME:
        {
          cout << IdString(di.kv.id);
          break;
        }
        case DELTA_SEQ_EVR:
        {
          cout << IdString(di.kv.id);
          break;
        }
        case DELTA_SEQ_NUM:
        {
          cout << di.kv.str;
          break;
        }
        default:
          cout << "ingoring " << IdString(di.key->name) << endl;
        }
        cout << endl;
      }
    }
  }

  PoolQuery q;
  q.addKind(ResKind::package);
  q.addAttribute(sat::SolvAttr::name, "libzypp");
  q.setMatchExact();
  
  std::for_each(q.begin(), q.end(), PrintAndCount());

  PoolItem pi(*q.poolItemBegin());
  if (pi)
  {
    Package::constPtr p = asKind<Package>(pi.resolvable());

    std::list<Repository> repos( pool.reposBegin(), pool.reposEnd() );
    repo::DeltaCandidates deltas(repos, p->name());
    deltas.deltaRpms(p);
  }
  else
    cout << "no such package" << endl;
}
