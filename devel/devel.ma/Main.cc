#include "Tools.h"
#include "FakePool.h"

#include "zypp/base/Exception.h"
#include "zypp/base/InputStream.h"
#include "zypp/base/DefaultIntegral.h"
#include <zypp/base/Function.h>
#include <zypp/base/Iterator.h>

#include <zypp/SourceManager.h>
#include <zypp/SourceFactory.h>
#include "zypp/ZYppCallbacks.h"

#include "zypp/NVRAD.h"
#include "zypp/ResFilters.h"
#include "zypp/CapFilters.h"
#include "zypp/PackageKeyword.h"
#include "zypp/pool/GetResolvablesToInsDel.h"

using namespace std;
using namespace zypp;

///////////////////////////////////////////////////////////////////

struct SetTransactValue
{
  SetTransactValue( ResStatus::TransactValue newVal_r, ResStatus::TransactByValue causer_r )
  : _newVal( newVal_r )
  , _causer( causer_r )
  {}

  ResStatus::TransactValue   _newVal;
  ResStatus::TransactByValue _causer;

  bool operator()( const PoolItem & pi ) const
  {
    bool ret = pi.status().setTransactValue( _newVal, _causer );
    if ( ! ret )
      ERR << _newVal <<  _causer << " " << pi << endl;
    return ret;
  }
};

struct StatusReset : public SetTransactValue
{
  StatusReset()
  : SetTransactValue( ResStatus::KEEP_STATE, ResStatus::USER )
  {}
};

struct StatusInstall : public SetTransactValue
{
  StatusInstall()
  : SetTransactValue( ResStatus::TRANSACT, ResStatus::USER )
  {}
};

inline bool selectForTransact( const NameKindProxy & nkp, Arch arch = Arch() )
{
  if ( nkp.availableEmpty() )
  {
    ERR << "No Item to select: " << nkp << endl;
    return false;
    ZYPP_THROW( Exception("No Item to select") );
  }

  if ( arch != Arch() )
  {
    typeof( nkp.availableBegin() ) it =  nkp.availableBegin();
    for ( ; it != nkp.availableEnd(); ++it )
    {
      if ( (*it)->arch() == arch )
	return (*it).status().setTransact( true, ResStatus::USER );
    }
  }

  return nkp.availableBegin()->status().setTransact( true, ResStatus::USER );
}

///////////////////////////////////////////////////////////////////

bool solve( bool establish = false )
{
  if ( establish )
  {
    bool eres = getZYpp()->resolver()->establishPool();
    if ( ! eres )
    {
      ERR << "establish " << eres << endl;
      return false;
    }
    MIL << "establish " << eres << endl;
  }

  bool rres = getZYpp()->resolver()->resolvePool();
  if ( ! rres )
  {
    ERR << "resolve " << rres << endl;
    return false;
  }
  MIL << "resolve " << rres << endl;
  return true;
}

///////////////////////////////////////////////////////////////////

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;
  ///////////////////////////////////////////////////////////////////
  // define pool
  ///////////////////////////////////////////////////////////////////
  if ( 0 )
  {
    const char * data[] = {
      "@ product",
      "@ installed",
      "- prodold 1 1 x86_64",
      "@ available",
      "- prodnew 1 1 x86_64",
      "@ obsoletes",
      "prodold",
      "@ fin",
    };
    debug::addPool( data, data + ( sizeof(data) / sizeof(const char *) ) );
  }
  else
  {
    debug::addPool( "SRC/iorderbug.pool" );
  }
  ResPool pool( getZYpp()->pool() );
  vdumpPoolStats( USR << "Initial pool:" << endl,
		  pool.begin(),
		  pool.end() ) << endl;

  ///////////////////////////////////////////////////////////////////
  // define transaction
  ///////////////////////////////////////////////////////////////////
  if ( 0 )
    for_each( pool.byKindBegin<Product>(), pool.byKindEnd<Product>(), StatusInstall() );

#define selt(K,N) selectForTransact( nameKindProxy<K>( pool, #N ) )
  selt( Package, bash );
  selt( Package, readline );
  selt( Package, fontcfg );
#undef selt

  ///////////////////////////////////////////////////////////////////
  // solve
  ///////////////////////////////////////////////////////////////////
  if ( 1 )
  {
    solve();
  }

  vdumpPoolStats( USR << "Transacting:"<< endl,
		  make_filter_begin<resfilter::ByTransact>(pool),
		  make_filter_end<resfilter::ByTransact>(pool) ) << endl;

  ///////////////////////////////////////////////////////////////////
  // install order
  ///////////////////////////////////////////////////////////////////
  pool::GetResolvablesToInsDel collect( pool, pool::GetResolvablesToInsDel::ORDER_BY_MEDIANR );
  MIL << "GetResolvablesToInsDel:" << endl << collect << endl;

  if ( 1 )
  {
    // Collect until the 1st package from an unwanted media occurs.
    // Further collection could violate install order.
    bool hitUnwantedMedia = false;
    typedef pool::GetResolvablesToInsDel::PoolItemList PoolItemList;
    PoolItemList::iterator fst=collect._toInstall.end();
    for ( PoolItemList::iterator it = collect._toInstall.begin(); it != collect._toInstall.end(); ++it)
    {
      ResObject::constPtr res( it->resolvable() );

      if ( hitUnwantedMedia
	   || ( res->sourceMediaNr() && res->sourceMediaNr() != 1 ) )
      {
	if ( !hitUnwantedMedia )
	  fst=it;
	hitUnwantedMedia = true;
      }
      else
      {
      }
    }
    dumpRange( WAR << "toInstall1: " << endl,
	       collect._toInstall.begin(), fst ) << endl;
    dumpRange( WAR << "toInstall2: " << endl,
	       fst, collect._toInstall.end() ) << endl;
    dumpRange( ERR << "toDelete: " << endl,
	       collect._toDelete.begin(), collect._toDelete.end() ) << endl;
  }
  else
  {
    dumpRange( WAR << "toInstall: " << endl,
	       collect._toInstall.begin(), collect._toInstall.end() ) << endl;
    dumpRange( ERR << "toDelete: " << endl,
	       collect._toDelete.begin(), collect._toDelete.end() ) << endl;
  }

  INT << "===[END]============================================" << endl << endl;
  zypp::base::LogControl::instance().logNothing();
  return 0;
}

