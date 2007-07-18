/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */

#include <iostream>

#include <zypp/ZYpp.h>
#include <zypp/zypp_detail/ZYppReadOnlyHack.h>
#include <zypp/ZYppFactory.h>
#include <zypp/RepoManager.h>
#include <zypp/base/Logger.h>
#include <zypp/base/Exception.h>
#include <zypp/base/Algorithm.h>
#include <zypp/Product.h>

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "zypp-query-pool"

#include "keyring-callbacks.h"

using namespace std;
using namespace zypp;

//-----------------------------------------------------------------------------

class PrintItem : public resfilter::PoolItemFilterFunctor
{
public:
  string _repository;

  PrintItem( const string & repository )
      : _repository( repository )
  { }

  bool operator()( PoolItem_Ref item )
  {
    if (_repository.empty()
        || _repository == item->repository().info().alias())
    {
      cout << (item.status().isInstalled() ? "i" : " ");
      cout << "|" << item->kind();
      cout << "|" << item->name();
      cout << "|" << item->edition().version();
      if (!item->edition().release().empty())
        cout << "-" << item->edition().release();
      cout << "|" << item->arch();

      if ( isKind<Product>( item.resolvable() ) )
        {
          Product::constPtr p( asKind<Product>( item ) );
          cout << "|" << p->distributionName();
          cout << "|" << p->distributionEdition();
        }
      cout << endl;
    }
    return true;
  }
};




static void
query_pool( ZYpp::Ptr Z, string filter, const string & repository)
{
  Resolvable::Kind kind;

#define FILTER_ALL "all"
  if ( filter.empty() ) filter = FILTER_ALL;

  if (filter == "packages") kind = ResTraits<zypp::Package>::kind;
  else if (filter == "patches") kind = ResTraits<zypp::Patch>::kind;
  else if (filter == "patterns") kind = ResTraits<zypp::Pattern>::kind;
  else if (filter == "selections") kind = ResTraits<zypp::Selection>::kind;
  else if (filter == "products") kind = ResTraits<zypp::Product>::kind;
  else if (filter != FILTER_ALL)
  {
    std::cerr << "usage: zypp-query-pool [packages|patches|patterns|products] [<alias>]" << endl;
    exit( 1 );
  }

  bool system = (repository == "@system");

  MIL << "query_pool kind '" << kind << "', repository '" << repository << "'" << endl;

  if (!system)
  {
    try
    {
      MIL << "Load enabled repositories..." << endl;
      RepoManager  repoManager;
      RepoInfoList repos = repoManager.knownRepositories();
      for ( RepoInfoList::iterator it = repos.begin(); it != repos.end(); ++it )
      {
        RepoInfo & repo( *it );

        if ( ! repo.enabled() )
          continue;

        MIL << "Loading " << repo << endl;
        if ( ! repoManager.isCached( repo ) )
        {
          MIL << "Must build cache..." << repo << endl;
          repoManager.buildCache( repo );
        }
        Z->addResolvables( repoManager.createFromCache( repo ).resolvables() );
      }
      MIL << "Loaded enabled repositories." << endl;
    }
    catch (Exception & excpt_r)
    {
      ZYPP_CAUGHT( excpt_r );
      ERR << "Couldn't restore sources" << endl;
      exit( 1 );
    }
  }

  // add resolvables from the system
  if ( filter != FILTER_ALL )
  {
    MIL << "Loading target (" << kind << ")..." << endl;
    ResStore items;
    for (ResStore::resfilter_const_iterator it = Z->target()->byKindBegin(kind); it != Z->target()->byKindEnd(kind); ++it)
    {
      SEC << *it << endl;
      items.insert(*it);
    }
    Z->addResolvables( items, true );
  }
  else
  {
    // no filter, just add themm all
    MIL << "Loading target..." << endl;
    Z->addResolvables( Z->target()->resolvables(), true );
  }
  MIL << "Loaded target." << endl;


  MIL << "Pool has " << Z->pool().size() << " entries" << endl;
  if ( filter == FILTER_ALL)
  {
    PrintItem printitem( system ? "" : repository );
    if (system)
      zypp::invokeOnEach( Z->pool().begin(), Z->pool().end(),				// all kinds
                          zypp::resfilter::ByInstalled(),
                          zypp::functor::functorRef<bool,PoolItem> (printitem) );
    else
      zypp::invokeOnEach( Z->pool().begin(), Z->pool().end(),				// all kinds
                          zypp::functor::functorRef<bool,PoolItem> (printitem) );

  }
  else
  {
    PrintItem printitem( system ? "" : repository );
    if (system)
      zypp::invokeOnEach( Z->pool().byKindBegin( kind ), Z->pool().byKindEnd( kind ),	// filter kind
                          zypp::resfilter::ByInstalled(),
                          zypp::functor::functorRef<bool,PoolItem> (printitem) );
    else
      zypp::invokeOnEach( Z->pool().byKindBegin( kind ), Z->pool().byKindEnd( kind ),	// filter kind
                          zypp::functor::functorRef<bool,PoolItem> (printitem) );
  }
  return;
}

//-----------------------------------------------------------------------------

int
main (int argc, char **argv)
{
  MIL << "-------------------------------------" << endl;
  string filter;
  if (argc > 1)
    filter = argv[1];
  string repository;
  if (argc > 2)
    repository = argv[2];

  MIL << "START zypp-query-pool " << filter << " " << repository << endl;

  zypp::zypp_readonly_hack::IWantIt();
  ZYpp::Ptr Z = zypp::getZYpp();

  KeyRingCallbacks keyring_callbacks;
  DigestCallbacks digest_callbacks;

  Z->initializeTarget( "/" );

  query_pool( Z, filter, repository );

  MIL << "END zypp-query-pool, result 0" << endl;

  return 0;
}
