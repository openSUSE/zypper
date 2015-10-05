#include "Tools.h"

#include "zypp/PoolQueryResult.h"

#include <zypp/base/PtrTypes.h>
#include <zypp/base/Exception.h>
#include <zypp/base/Gettext.h>
#include <zypp/base/LogTools.h>
#include <zypp/base/Debug.h>
#include <zypp/base/Functional.h>
#include <zypp/base/IOStream.h>
#include <zypp/base/InputStream.h>
#include <zypp/base/ProvideNumericId.h>
#include <zypp/base/Flags.h>
#include <zypp/AutoDispose.h>

#include "zypp/ResPoolProxy.h"

#include "zypp/ZYppCallbacks.h"
#include "zypp/ResPool.h"
#include "zypp/ResFilters.h"
#include "zypp/ResObjects.h"
#include "zypp/Digest.h"
#include "zypp/PackageKeyword.h"
#include "zypp/TmpPath.h"
#include "zypp/ManagedFile.h"


#include "zypp/RepoManager.h"
#include "zypp/Repository.h"
#include "zypp/RepoInfo.h"

#include "zypp/repo/PackageProvider.h"

#include "zypp/ResPoolProxy.h"

#include "zypp/sat/Pool.h"
#include "zypp/sat/LocaleSupport.h"
#include "zypp/sat/LookupAttr.h"
#include "zypp/sat/SolvableSet.h"
#include "zypp/sat/SolvIterMixin.h"
#include "zypp/sat/detail/PoolImpl.h"
#include "zypp/sat/WhatObsoletes.h"
#include "zypp/PoolQuery.h"
#include "zypp/ServiceInfo.h"

#include <boost/mpl/int.hpp>

using namespace std;
using namespace zypp;
using namespace zypp::functor;
using namespace zypp::ui;

///////////////////////////////////////////////////////////////////

static const Pathname sysRoot( getenv("SYSROOT") ? getenv("SYSROOT") : "/Local/ROOT" );

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

bool queryInstalledEditionHelper( const std::string & name_r,
                                  const Edition &     ed_r,
                                  const Arch &        arch_r )
{
  if ( ed_r == Edition::noedition )
    return true;
  if ( name_r == "kernel-default" && ed_r == Edition("2.6.22.5-10") )
    return true;
  if ( name_r == "update-test-affects-package-manager" && ed_r == Edition("1.1-6") )
    return true;

  return false;
}


ManagedFile repoProvidePackage( const PoolItem & pi )
{
  ResPool _pool( getZYpp()->pool() );
  repo::RepoMediaAccess _access;

  // Redirect PackageProvider queries for installed editions
  // (in case of patch/delta rpm processing) to rpmDb.
  repo::PackageProviderPolicy packageProviderPolicy;
  packageProviderPolicy.queryInstalledCB( queryInstalledEditionHelper );

  Package::constPtr p = asKind<Package>(pi.resolvable());

  // Build a repository list for repos
  // contributing to the pool
  repo::DeltaCandidates deltas( repo::makeDeltaCandidates( _pool.knownRepositoriesBegin(),
                                                           _pool.knownRepositoriesEnd() ) );
  repo::PackageProvider pkgProvider( _access, p, deltas, packageProviderPolicy );
  return pkgProvider.providePackage();
}

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

namespace zypp
{
  template <class TLIterator, class TRIterator, class TFunction>
      inline int invokeOnEach( TLIterator lbegin_r, TLIterator lend_r,
                               TRIterator rbegin_r, TRIterator rend_r,
                               TFunction fnc_r )
      {
        int cnt = 0;
        for ( TLIterator lit = lbegin_r; lit != lend_r; ++lit )
        {
          for ( TRIterator rit = rbegin_r; rit != rend_r; ++rit )
          {
            ++cnt;
            if ( ! fnc_r( *lit, *rit ) )
              return -cnt;
          }
        }
        return cnt;
      }
}

///////////////////////////////////////////////////////////////////

std::ostream & testDump( std::ostream & str, const PoolItem & pi )
{
  str << pi << endl;
  Package::constPtr p( asKind<Package>(pi) );
  if ( p )
  {
#define OUTS(V) str << str::form("%-25s: ",#V) << p->V() << endl
    Locale l( "de" );
    str << str::form("%-25s: ",l.code().c_str()) << p->summary(l) << endl;
    l = Locale( "fr" );
    str << str::form("%-25s: ",l.code().c_str()) << p->summary(l) << endl;
    l = Locale( "dsdf" );
    str << str::form("%-25s: ",l.code().c_str()) << p->summary(l) << endl;
    OUTS( summary );
    OUTS( installSize );
    OUTS( downloadSize );
    OUTS( sourcePkgName );
    OUTS( sourcePkgEdition );
    OUTS( checksum );
    OUTS( location );
#undef OUTS


  }
  return str;
}

struct Xprint
{
  bool operator()( const PoolItem & obj_r )
  {
    //MIL << obj_r << endl;
    //DBG << " -> " << obj_r->satSolvable() << endl;

    return true;
  }

  bool operator()( const sat::Solvable & obj_r )
  {
    //dumpOn( MIL, obj_r ) << endl;
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

///////////////////////////////////////////////////////////////////

bool solve()
{
  bool rres = false;
  {
    //zypp::base::LogControl::TmpLineWriter shutUp;
    rres = getZYpp()->resolver()->resolvePool();
  }
  if ( ! rres )
  {
    ERR << "resolve " << rres << endl;
    getZYpp()->resolver()->problems();
    return false;
  }
  MIL << "resolve " << rres << endl;
  return true;
}

bool install()
{
  ZYppCommitPolicy pol;
  pol.dryRun(true);
  pol.rpmInstFlags( pol.rpmInstFlags().setFlag( target::rpm::RPMINST_JUSTDB ) );
  SEC << getZYpp()->commit( pol ) << endl;
  return true;
}

void testcase()
{
  getZYpp()->resolver()->createSolverTestcase( "./solverTestcase" );
}

///////////////////////////////////////////////////////////////////

struct DigestReceive : public callback::ReceiveReport<DigestReport>
{
  DigestReceive()
  {
    connect();
  }

  virtual bool askUserToAcceptNoDigest( const zypp::Pathname &file )
  {
    USR << endl;
    return false;
  }
  virtual bool askUserToAccepUnknownDigest( const Pathname &file, const std::string &name )
  {
    USR << endl;
    return false;
  }
  virtual bool askUserToAcceptWrongDigest( const Pathname &file, const std::string &requested, const std::string &found )
  {
    USR << "fle " << PathInfo(file) << endl;
    USR << "req " << requested << endl;
    USR << "fnd " << found << endl;
    return false;
  }
};

struct KeyRingSignalsReceive : public callback::ReceiveReport<KeyRingSignals>
{
  KeyRingSignalsReceive()
  {
    connect();
  }
  virtual void trustedKeyAdded( const PublicKey &/*key*/ )
  {
    USR << endl;
  }
  virtual void trustedKeyRemoved( const PublicKey &/*key*/ )
  {
    USR << endl;
  }
};

///////////////////////////////////////////////////////////////////

struct MediaChangeReceive : public callback::ReceiveReport<media::MediaChangeReport>
{
  virtual Action requestMedia( Url & source
                               , unsigned mediumNr
                               , const std::string & label
                               , Error error
                               , const std::string & description
                               , const std::vector<std::string> & devices
                               , unsigned int & dev_current )
  {
    SEC << __FUNCTION__ << endl
    << "  " << source << endl
    << "  " << mediumNr << endl
    << "  " << label << endl
    << "  " << error << endl
    << "  " << description << endl
    << "  " << devices << endl
    << "  " << dev_current << endl;
    return IGNORE;
  }
};

///////////////////////////////////////////////////////////////////

namespace container
{
  template<class Tp>
    bool isIn( const std::set<Tp> & cont, const typename std::set<Tp>::value_type & val )
    { return cont.find( val ) != cont.end(); }
}
///////////////////////////////////////////////////////////////////

void itCmp( const sat::Pool::SolvableIterator & l, const sat::Pool::SolvableIterator & r )
{
  SEC << *l << " - " << *r << endl;
  INT << "== " << (l==r) << endl;
  INT << "!= " << (l!=r) << endl;
}

bool isTrue()  { return true; }
bool isFalse() { return false; }

void dumpIdStr()
{
  for ( int i = -3; i < 30; ++i )
  {
    DBG << i << '\t' << IdString( i ) << endl;
  }
}

void ttt( const char * lhs, const char * rhs )
{
  DBG << lhs << " <=> " << rhs << " --> " << ::strcmp( lhs, rhs ) << endl;
}

namespace zypp
{
namespace filter
{
  template <class TMemFun, class TValue>
  class HasValue
  {
    public:
      HasValue( TMemFun fun_r, TValue val_r )
      : _fun( fun_r ), _val( val_r )
      {}
      template <class Tp>
      bool operator()( const Tp & obj_r ) const
      { return( _fun && (obj_r.*_fun)() == _val ); }
    private:
      TMemFun _fun;
      TValue  _val;
  };

  template <class TMemFun, class TValue>
  HasValue<TMemFun, TValue> byValue( TMemFun fun_r, TValue val_r )
  { return HasValue<TMemFun, TValue>( fun_r, val_r ); }
}

}

template <class L>
struct TestO { TestO( const L & lhs ) : _lhs( lhs ) {} const L & _lhs; };

template <class L>
std::ostream & operator<<( std::ostream & str, const TestO<L> & obj )
{ const L & lhs( obj._lhs); return str << (lhs?'_':'*') << (lhs.empty()?'e':'_') << "'" << lhs << "'"; }

template <class L>
TestO<L> testO( const L & lhs )
{ return TestO<L>( lhs ); }

template <class L, class R>
void testCMP( const L & lhs, const R & rhs )
{
  MIL << "LHS " << testO(lhs) << endl;
  MIL << "RHS " << rhs << endl;

#define OUTS(S) DBG << #S << ": " << (S) << endl
  OUTS( lhs.compare(rhs) );
  OUTS( lhs != rhs );
  OUTS( lhs <  rhs );
  OUTS( lhs <= rhs );
  OUTS( lhs == rhs );
  OUTS( lhs >= rhs );
  OUTS( lhs >  rhs );
#undef OUTS
}

inline bool useRepo( RepoInfo & repo )
{
  //return repo.alias()  == "matest";
  return repo.enabled();
}

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
try {
  --argc;
  ++argv;
  zypp::base::LogControl::instance().logToStdErr();
  INT << "===[START]==========================================" << endl;
  ZConfig::instance();

  ResPool   pool( ResPool::instance() );
  sat::Pool satpool( sat::Pool::instance() );

  if ( 0 )
  {
    Measure x( "INIT TARGET" );
    {
      {
        zypp::base::LogControl::TmpLineWriter shutUp;
        getZYpp()->initializeTarget( sysRoot );
      }
      getZYpp()->target()->load();
      USR << getZYpp()->target()->targetDistribution() << endl;
      USR << getZYpp()->target()->targetDistributionRelease() << endl;
    }
  }

  if ( 1 )
  {
    RepoManager repoManager( makeRepoManager( sysRoot ) );
    RepoInfoList repos = repoManager.knownRepositories();

    // launch repos
    for ( RepoInfoList::iterator it = repos.begin(); it != repos.end(); ++it )
    {
      RepoInfo & nrepo( *it );
      SEC << nrepo << endl;

      if ( ! useRepo( nrepo ) )
        continue;

      if ( ! repoManager.isCached( nrepo ) || nrepo.type() == repo::RepoType::RPMPLAINDIR )
      {
        if ( repoManager.isCached( nrepo ) )
        {
          SEC << "cleanCache" << endl;
          repoManager.cleanCache( nrepo );
        }
        SEC << "refreshMetadata" << endl;
        //repoManager.refreshMetadata( nrepo );
        SEC << "buildCache" << endl;
        repoManager.buildCache( nrepo );
      }
    }

    // create from cache:
    {
      Measure x( "CREATE FROM CACHE" );
      for ( RepoInfoList::iterator it = repos.begin(); it != repos.end(); ++it )
      {
        RepoInfo & nrepo( *it );
        if ( ! useRepo( nrepo ) )
          continue;

        Measure x( "CREATE FROM CACHE "+nrepo.alias() );
        try
        {
          repoManager.loadFromCache( nrepo );
        }
        catch ( const Exception & exp )
        {
          MIL << "Try to rebuild cache..." << endl;
          SEC << "cleanCache" << endl;
          repoManager.cleanCache( nrepo );
          SEC << "buildCache" << endl;
          repoManager.buildCache( nrepo );
          SEC << "Create from cache" << endl;
          repoManager.loadFromCache( nrepo );
        }

        USR << "pool: " << pool << endl;
      }
    }
  }

  dumpRange( USR, satpool.reposBegin(), satpool.reposEnd() );
  USR << "pool: " << pool << endl;

  ///////////////////////////////////////////////////////////////////

  if ( 0 )
  {
    Measure x( "Upgrade" );
    getZYpp()->resolver()->doUpgrade();
  }

  ///////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
//  UNTh_(2)product:openSUSE-11.1.x86_64(openSUSE-dvd-11.1)
//  U_Th_(583)yast2-ntp-client-2.17.1-1.26.noarch(openSUSE-dvd-11.1)
//  U_Th_(1652)kernel-default-2.6.27-7.2.x86_64(openSUSE-dvd-11.1)
//  U_Th_(2490)ntp-4.2.4p5-1.6.x86_64(openSUSE-dvd-11.1)
//  UNTh_(2545)openSUSE-release-11.1-1.4.x86_64(openSUSE-dvd-11.1)
//  USTh_(3462)pattern:base-11.1-46.1.x86_64(openSUSE-dvd-11.1)
//  USTh_(3672)pattern:x11-11.1-46.1.x86_64(openSUSE-dvd-11.1)
//  USTu_(3680)pattern:xfce-11.1-46.1.x86_64(openSUSE-dvd-11.1)


  getZYpp()->resolver()->addRequire( Capability("product:openSUSE") );
  getZYpp()->resolver()->addRequire( Capability("yast2-ntp-client") );
  getZYpp()->resolver()->addRequire( Capability("kernel-default") );
  getZYpp()->resolver()->addRequire( Capability("ntp") );
  getZYpp()->resolver()->addRequire( Capability("openSUSE-release") );
  getZYpp()->resolver()->addRequire( Capability("pattern:base") );
  getZYpp()->resolver()->addRequire( Capability("pattern:x11") );
  getZYpp()->resolver()->addRequire( Capability("pattern:xfce") );

  solve();

  vdumpPoolStats( USR << "Transacting:"<< endl,
                  make_filter_begin<resfilter::ByTransact>(pool),
                  make_filter_end<resfilter::ByTransact>(pool) ) << endl;

  ByteCount sze;
  ByteCount dusze;
  DiskUsageCounter ducounter( DiskUsageCounter::justRootPartition() );
  for_( it, make_filter_begin<resfilter::ByTransact>(pool), make_filter_end<resfilter::ByTransact>(pool) )
  {
    USR << *it << endl;
    ByteCount csze( ducounter.disk_usage( *it ).begin()->commitDiff() );
    sze += (*it)->installSize();
    dusze += csze;
    DBG <<(*it)->installSize() << " vs. " << csze << " | " << ByteCount( dusze-sze ) << endl;

  }
  SEC << sze << endl;
  SEC << dusze << endl;

  SEC << zypp::getZYpp()->diskUsage() << endl;

  ///////////////////////////////////////////////////////////////////
  INT << "===[END]============================================" << endl << endl;
  zypp::base::LogControl::instance().logNothing();
  return 0;
}
catch ( const Exception & exp )
{
  INT << exp << endl << exp.historyAsString();
}
catch (...)
{}


