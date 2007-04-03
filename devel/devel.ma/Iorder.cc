#include <ctime>
#include <iostream>
#include <fstream>
#include "Tools.h"

#include <zypp/base/PtrTypes.h>
#include <zypp/base/Exception.h>
#include <zypp/base/ProvideNumericId.h>
#include <zypp/base/DefaultIntegral.h>

using namespace zypp;
#include "FakePool.h"

#include "zypp/ZYppFactory.h"
#include "zypp/ResPoolProxy.h"
#include <zypp/SourceManager.h>
#include <zypp/SourceFactory.h>
#include <zypp/VendorAttr.h>

#include "zypp/NVRAD.h"
#include "zypp/ResTraits.h"
#include "zypp/ResPool.h"
#include "zypp/ResFilters.h"
#include "zypp/CapFilters.h"
#include "zypp/CapFactory.h"
#include "zypp/Package.h"
#include "zypp/Language.h"
#include "zypp/NameKindProxy.h"
#include "zypp/pool/GetResolvablesToInsDel.h"
#include "zypp/source/PackageProvider.h"
#include "zypp/target/rpm/RpmDb.h"
#include "zypp/TmpPath.h"
#include "zypp/target/CommitPackageCache.h"

using namespace std;
using namespace zypp;
using namespace zypp::ui;
using namespace zypp::functor;
using namespace zypp::debug;
using namespace zypp::target;
using namespace zypp::target::rpm;

///////////////////////////////////////////////////////////////////

static const Pathname sysRoot( "/Local/ROOT" );

///////////////////////////////////////////////////////////////////

struct AddResolvables
{
  bool operator()( const Source_Ref & src ) const
  {
    getZYpp()->addResolvables( src.resolvables() );
    return true;
  }
};

struct AddSource
{
  bool operator()( const std::string & url )
  {
    SourceManager::sourceManager()->addSource( createSource( url ) );
    return true;
  }
};
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
  if ( nkp.availableEmpty() ) {
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

#include "zypp/CapMatchHelper.h"

struct GetObsoletes
{
  void operator()( const PoolItem & pi )
  {
    INT << pi << endl;
    for_each( pi->dep(Dep::OBSOLETES).begin(),
              pi->dep(Dep::OBSOLETES).end(),
              ForEachMatchInPool( getZYpp()->pool(), Dep::PROVIDES,
                                  Print() ) );
  }
};

///////////////////////////////////////////////////////////////////

template<class _Iterator>
  void addPool( _Iterator begin_r, _Iterator end_r )
  {
    DataCollect dataCollect;
    dataCollect.collect( begin_r, end_r );
    getZYpp()->addResolvables( dataCollect.installed(), true );
    getZYpp()->addResolvables( dataCollect.available() );
    vdumpPoolStats( USR << "Pool:" << endl,
                    getZYpp()->pool().begin(),
                    getZYpp()->pool().end() ) << endl;
  }

template<class _Res>
  void poolRequire( const std::string & capstr_r,
                    ResStatus::TransactByValue causer_r = ResStatus::USER )
  {
    getZYpp()->pool().additionalRequire()[causer_r]
                     .insert( CapFactory().parse( ResTraits<_Res>::kind,
                                                  capstr_r ) );
  }

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

      struct StorageRemoveObsoleted
      {
        StorageRemoveObsoleted( const PoolItem & byPoolitem_r )
        : _byPoolitem( byPoolitem_r )
        {}

        bool operator()( const PoolItem & poolitem_r ) const
        {
          if ( ! poolitem_r.status().isInstalled() )
            return true;

          if ( isKind<Package>(poolitem_r.resolvable()) )
            {
              ERR << "Ignore unsupported Package/non-Package obsolete: "
                  << _byPoolitem << " obsoletes " << poolitem_r << endl;
              return true;
            }

          INT << poolitem_r << " by " << _byPoolitem << endl;

          return true;
        }

      private:
        const PoolItem               _byPoolitem;
      };

///////////////////////////////////////////////////////////////////

/** Let the Source provide the package.
*/
static ManagedFile globalSourceProvidePackage( const PoolItem & pi )
{
  Package::constPtr p( asKind<Package>(pi.resolvable()) );
  string fakename( p->name() + "-" + p->edition().asString() + "." + p->arch().asString() + ".rpm" );
  Pathname fake( "/tmp/FAKEPKG/" + fakename );

  if ( 0 )
    {
      std::ofstream x( fake.c_str() );
      x << p << endl;
      WAR << "GEN package " << x << endl;
    }

  DBG << "SOURCE PACKAGE PROVIDE " << fake << endl;
  return fake;
}

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  //zypp::base::LogControl::instance().logfile( "log.restrict" );
  INT << "===[START]==========================================" << endl;

  ResPool pool( getZYpp()->pool() );

  if ( 1 )
    {
      zypp::base::LogControl::TmpLineWriter shutUp;
      getZYpp()->initTarget( sysRoot );
    }
  MIL << "Added target: " << pool << endl;

  if ( 1 )
    {
      zypp::base::LogControl::TmpLineWriter shutUp;
      Source_Ref src;
      src = createSource( "dir:/Local/dist/install/SLP/SLES-10-SP1-Build00052/i386/CD1",
                          "Addon-BiArch" );
      getZYpp()->addResolvables( src.resolvables() );
    }
  MIL << "Added sources: " << pool << endl;

  // select...
  for_each( pool.byKindBegin<Product>(), pool.byKindEnd<Product>(), StatusInstall() );
#define selt(K,N) selectForTransact( nameKindProxy<K>( pool, #N ) )
//   selt( Selection, default );
//   selt( Package, RealPlayer );
//   selt( Package, acroread );
//   selt( Package, flash-player );
  selt( Package, fontconfig );
#undef selt


  solve();
  //vdumpPoolStats( USR << "Transacting:"<< endl,
  //                make_filter_begin<resfilter::ByTransact>(pool),
  //                make_filter_end<resfilter::ByTransact>(pool) ) << endl;
  pool::GetResolvablesToInsDel collect( pool, pool::GetResolvablesToInsDel::ORDER_BY_MEDIANR );
  MIL << "GetResolvablesToInsDel:" << endl << collect << endl;

  typedef pool::GetResolvablesToInsDel::PoolItemList PoolItemList;

  if ( 1 )
    {
      // Collect until the 1st package from an unwanted media occurs.
      // Further collection could violate install order.
      bool hitUnwantedMedia = false;
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

