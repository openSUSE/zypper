#include <iostream>
#include "Tools.h"

#include <zypp/base/LogControl.h>
#include <zypp/base/LogTools.h>
#include <zypp/base/IOStream.h>
#include <zypp/base/Algorithm.h>
#include <zypp/base/PtrTypes.h>
#include <zypp/base/Exception.h>
#include <zypp/base/ProvideNumericId.h>


#include "zypp/ZYppFactory.h"
#include "zypp/ResPoolProxy.h"
#include <zypp/SourceManager.h>
#include <zypp/SourceFactory.h>

#include "zypp/ZYppCallbacks.h"
#include "zypp/NVRAD.h"
#include "zypp/ResPool.h"
#include "zypp/ResFilters.h"
#include "zypp/CapFilters.h"
#include "zypp/Package.h"
#include "zypp/Pattern.h"
#include "zypp/Language.h"
#include "zypp/NameKindProxy.h"
#include "zypp/pool/GetResolvablesToInsDel.h"
#include <zypp/Source.h>
#include <zypp/ResStore.h>
#include <zypp/ResObject.h>
#include <zypp/pool/PoolStats.h>
#include <zypp/KeyRing.h>
#include <zypp/ResPoolProxy.h>

using namespace std;
using namespace zypp;
using namespace zypp::ui;
using namespace zypp::functor;

///////////////////////////////////////////////////////////////////

static const Pathname sysRoot( "/Local/ROOT" );

///////////////////////////////////////////////////////////////////

template<class _Condition>
  struct SetTrue
  {
    SetTrue( _Condition cond_r )
    : _cond( cond_r )
    {}

    template<class _Tp>
      bool operator()( _Tp t ) const
      {
        _cond( t );
        return true;
      }

    _Condition _cond;
  };

template<class _Condition>
  inline SetTrue<_Condition> setTrue_c( _Condition cond_r )
  {
    return SetTrue<_Condition>( cond_r );
  }

struct PrintPoolItem
{
  void operator()( const PoolItem & pi ) const
  { USR << pi << " (" << pi.resolvable().get() << ")" <<endl; }
};

template <class _Iterator>
  std::ostream & vdumpPoolStats( std::ostream & str,
                                 _Iterator begin_r, _Iterator end_r )
  {
    pool::PoolStats stats;
    std::for_each( begin_r, end_r,
                   functor::chain( setTrue_c(PrintPoolItem()),
                                   setTrue_c(functor::functorRef<void,ResObject::constPtr>(stats)) )
                   );
    return str << stats;
  }

///////////////////////////////////////////////////////////////////

struct Dump
{
  template<class _Tp>
    bool operator()( const _Tp & o )
    {
      DBG << o << endl;
      return true;
    }
};

struct Select
{
  template<class _Tp>
    bool operator()( const _Tp & o )
    {
      if ( o->hasInstalledObj() && o->availableObjs() == 3 )
        SEC << o << endl;
      if ( o->name() == "db" )
        _res = o;
      return true;
    }

  Selectable::Ptr _res;
};

struct Test
{
  Test( Selectable::Ptr sel_r )
  : _sel( sel_r )
  , _i( _sel->installedPoolItem() )
  , _a( _sel->availablePoolItemBegin(), _sel->availablePoolItemEnd() )
  {}


  void dump()
  {
    DBG << str::form( "[%s]%s: ",
                      _sel->kind().asString().c_str(),
                      _sel->name().c_str() ) << _sel->status() << endl;

    WAR << "  i  " << _i << endl;

    for ( unsigned n = 0; n < _a.size(); ++n )
      {
        char c = ( _a[n] == _sel->candidatePoolItem() ? 'c' : ' ');
        INT << "  a" << c << " " << _a[n] << endl;
      }
  }

  Selectable::Ptr  _sel;
  PoolItem         _i;
  vector<PoolItem> _a;
};


/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  //zypp::base::LogControl::instance().logfile( "" );
  INT << "===[START]==========================================" << endl;

  ResPool pool( getZYpp()->pool() );

  if ( 1 )
    {
      zypp::base::LogControl::TmpLineWriter shutUp;

      getZYpp()->initializeTarget( sysRoot );
      getZYpp()->addResolvables( getZYpp()->target()->resolvables(), true );
    }

  Source_Ref src1;
  if ( 1 )
    {
      zypp::base::LogControl::TmpLineWriter shutUp;

      src1 = createSource( "dir:/Local/SLES10" );
      getZYpp()->addResolvables( src1.resolvables() );

      src1 = createSource( "dir:/Local/SUSE-Linux-10.1-Build_830-Addon-BiArch/CD1" );
      getZYpp()->addResolvables( src1.resolvables() );

      src1 = createSource( "dir:/Local/SUSE-Linux-10.1-Build_830-i386/CD1" );
      getZYpp()->addResolvables( src1.resolvables() );
    }

  //vdumpPoolStats( SEC, pool.begin(), pool.end() ) << endl;

  ResPoolProxy ui( getZYpp()->poolProxy() );
  Select sel = for_each( ui.byKindBegin<Package>(),
                         ui.byKindEnd<Package>(),
                         Select() );

#define T(N,V,C) \
  MIL << #N" "#V" "#C" " << t._a[N].status().setTransactValue( V, C ) << endl; \
  t.dump();

  Test t( sel._res );
  t.dump();

  T( 1, ResStatus::TRANSACT, ResStatus::SOLVER );
  T( 0, ResStatus::LOCKED, ResStatus::USER );
  T( 1, ResStatus::LOCKED, ResStatus::USER );
  T( 2, ResStatus::LOCKED, ResStatus::USER );
  DBG << t._sel->set_status( ui::S_Install ) << endl;
  DBG << t._sel->set_status( ui::S_Update ) << endl;
  t.dump();
  DBG << t._sel->set_status( ui::S_KeepInstalled ) << endl;
  t.dump();
  DBG << t._sel->set_status( ui::S_Taboo ) << endl;
  t.dump();
  DBG << t._sel->setCandidate( t._a[2] ) << endl;
  t.dump();
  DBG << t._sel->set_status( ui::S_Update ) << endl;
  t.dump();
  DBG << t._sel->setCandidate( t._a[1] ) << endl;
  t.dump();
  DBG << t._sel->set_status( ui::S_KeepInstalled ) << endl;
  t.dump();
  DBG << t._sel->setCandidate( t._a[2] ) << endl;
  t.dump();
  DBG << t._sel->set_status( ui::S_Update ) << endl;
  t.dump();
  DBG << t._sel->set_status( ui::S_KeepInstalled ) << endl;
  t.dump();
  DBG << t._sel->setCandidate( 0 ) << endl;
  t.dump();



  INT << "===[END]============================================" << endl << endl;
  new zypp::base::LogControl::TmpLineWriter;
  return 0;
}

