#include <ctime>
#include <iostream>
#include "Tools.h"

#include <zypp/base/PtrTypes.h>
#include <zypp/base/Exception.h>
#include <zypp/base/ProvideNumericId.h>

#include "zypp/ZYppFactory.h"
#include "zypp/ResPoolProxy.h"
#include <zypp/SourceManager.h>
#include <zypp/SourceFactory.h>

#include "zypp/NVRAD.h"
#include "zypp/ResPool.h"
#include "zypp/ResFilters.h"
#include "zypp/CapFilters.h"
#include "zypp/Package.h"
#include "zypp/Language.h"
#include "zypp/NameKindProxy.h"
#include "zypp/pool/GetResolvablesToInsDel.h"


using namespace std;
using namespace zypp;
using namespace zypp::ui;
using namespace zypp::functor;

///////////////////////////////////////////////////////////////////

static const Pathname sysRoot( "/Local/ROOT" );

///////////////////////////////////////////////////////////////////

namespace container
{
  template<class _Tp>
    bool isIn( const std::set<_Tp> & cont, const typename std::set<_Tp>::value_type & val )
    { return cont.find( val ) != cont.end(); }
}

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

template <class _Iterator, class _Filter, class _Function>
  inline _Function for_each_if( _Iterator begin_r, _Iterator end_r,
                                _Filter filter_r,
                                _Function fnc_r )
  {
    for ( _Iterator it = begin_r; it != end_r; ++it )
      {
        if ( filter_r( *it ) )
          {
            fnc_r( *it );
          }
      }
    return fnc_r;
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

struct PoolItemSelect
{
  void operator()( const PoolItem & pi ) const
  {
    if ( pi->source().numericId() == 2 )
      pi.status().setTransact( true, ResStatus::USER );
  }
};

///////////////////////////////////////////////////////////////////
typedef std::list<PoolItem> PoolItemList;
typedef std::set<PoolItem>  PoolItemSet;
#include "zypp/solver/detail/InstallOrder.h"
using zypp::solver::detail::InstallOrder;
#include "Iorder.h"

///////////////////////////////////////////////////////////////////

struct AddResolvables
{
  bool operator()( const Source_Ref & src ) const
  { getZYpp()->addResolvables( src.resolvables() ); }
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
  { return pi.status().setTransactValue( _newVal, _causer ); }
};

struct StatusReset : public SetTransactValue
{
  StatusReset()
  : SetTransactValue( ResStatus::KEEP_STATE, ResStatus::USER )
  {}
};


inline bool selectForTransact( const NameKindProxy & nkp )
{
  if ( nkp.availableEmpty() ) {
    ERR << "No Item to select: " << nkp << endl;
    return false;
    ZYPP_THROW( Exception("No Item to select") );
  }

  return nkp.availableBegin()->status().setTransact( true, ResStatus::USER );
}

///////////////////////////////////////////////////////////////////
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
      USR << "Added target: " << pool << endl;
    }

  if ( 1 ) {
    //zypp::base::LogControl::TmpLineWriter shutUp;
    SourceManager::sourceManager()->restore( sysRoot );
    if ( SourceManager::sourceManager()->allSources().empty() )
      {
        Source_Ref src1( createSource( "dir:///mounts/machcd2/CDs/SLES-10-CD-x86_64-Build_1304/CD1" ) );
        SourceManager::sourceManager()->addSource( src1 );
#if 0
        Source_Ref src2( createSource( "dir:///Local/SUSE-Linux-10.1-Build_830-Addon-BiArch/CD1" ) );
        SourceManager::sourceManager()->addSource( src2 );
#endif
        SourceManager::sourceManager()->store( sysRoot, true );
      }
    for_each( SourceManager::sourceManager()->Source_begin(), SourceManager::sourceManager()->Source_end(),
              AddResolvables() );
    dumpRange( USR << "Sources: ",
               SourceManager::sourceManager()->Source_begin(), SourceManager::sourceManager()->Source_end()
               ) << endl;
  }

  MIL << *SourceManager::sourceManager() << endl;
  MIL << pool << endl;

  if ( 1 )
    {
#if 0
2006-06-02 14:54:37 <1> 10.10.2.245(3269) [solver] Resolver.cc(resolvePool):947 Resolver::resolvePool()
2006-06-02 14:54:37 <0> 10.10.2.245(3269) [solver] Resolver.cc(show_pool):913 1: U_Tu_[S0:0][language]en_US-.noarch
2006-06-02 14:54:37 <0> 10.10.2.245(3269) [solver] Resolver.cc(show_pool):913 4: U_Th_[S2:0][product]SUSE-Linux-Enterprise-Server-ia64-10-0.ia64
2006-06-02 14:54:37 <0> 10.10.2.245(3269) [solver] Resolver.cc(show_pool):913 39: U_Th_[S2:0][pattern]apparmor-10-51.13.ia64
2006-06-02 14:54:37 <0> 10.10.2.245(3269) [solver] Resolver.cc(show_pool):913 50: U_Th_[S2:0][pattern]x86-10-51.13.ia64
2006-06-02 14:54:37 <0> 10.10.2.245(3269) [solver] Resolver.cc(show_pool):913 52: U_Th_[S2:0][pattern]base-10-51.13.ia64
2006-06-02 14:54:37 <0> 10.10.2.245(3269) [solver] Resolver.cc(show_pool):913 187: U_Th_[S2:1][package]fpswa-1.18-81.ia64
2006-06-02 14:54:37 <0> 10.10.2.245(3269) [solver] Resolver.cc(show_pool):913 226: U_Th_[S2:0][pattern]x11-10-51.13.ia64
2006-06-02 14:54:37 <0> 10.10.2.245(3269) [solver] Resolver.cc(show_pool):913 969: U_Th_[S2:1][package]kernel-default-2.6.16.18-1.4.ia64
2006-06-02 14:54:37 <0> 10.10.2.245(3269) [solver] Resolver.cc(show_pool):913 1995: U_Th_[S2:0][pattern]print_server-10-51.13.ia64
2006-06-02 14:54:37 <0> 10.10.2.245(3269) [solver] Resolver.cc(show_pool):913 2069: U_Th_[S2:0][pattern]gnome-10-51.13.ia64
2006-06-02 14:54:37 <0> 10.10.2.245(3269) [solver] Resolver.cc(show_pool):913 2130: U_Th_[S2:1][package]yast2-trans-en_US-2.13.5-7.1.noarch
2006-06-02 14:54:37 <1> 10.10.2.245(3269) [solver] Resolver.cc(resolveDependencies):606 Resolver::resolveDependencies()
#endif
#define selt(K,N) selectForTransact( nameKindProxy<K>( pool, #N ) )

      selt( Language, en_US );
      selt( Product, SUSE-Linux-Enterprise-Server-x86_64 );
      selt( Pattern, apparmor );
      //selt( Pattern, x86 );
      selt( Pattern, base );
      selt( Pattern, x11 );
      selt( Pattern, print_server );
      selt( Pattern, gnome );
      selt( Package, fpswa );
      selt( Package, kernel-default );
      selt( Package, yast2-trans-en_US );
    }
  else
    {
      selectForTransact( nameKindProxy<Selection>( pool, "default" ) );
      selectForTransact( nameKindProxy<Selection>( pool, "X11" ) );
      selectForTransact( nameKindProxy<Selection>( pool, "Kde" ) );
      selectForTransact( nameKindProxy<Selection>( pool, "Office" ) );
    }

  vdumpPoolStats( USR << "Transacting:"<< endl,
                  make_filter_begin<resfilter::ByTransact>(pool),
                  make_filter_end<resfilter::ByTransact>(pool) ) << endl;

  if ( 1 ) {
    bool eres, rres;
    {
      zypp::base::LogControl::TmpLineWriter shutUp;
      zypp::base::LogControl::instance().logfile( "SOLVER" );
      eres = getZYpp()->resolver()->establishPool();
      rres = getZYpp()->resolver()->resolvePool();
    }
    MIL << "est " << eres << " slv " << rres << endl;
  }

  dumpPoolStats( USR << "Transacting:"<< endl,
                  make_filter_begin<resfilter::ByTransact>(pool),
                  make_filter_end<resfilter::ByTransact>(pool) ) << endl;


  pool::GetResolvablesToInsDel collect( pool );
  MIL << "GetResolvablesToInsDel:" << endl << collect << endl;

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

