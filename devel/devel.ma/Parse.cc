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

#include "zypp/ZYppCallbacks.h"
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

struct ConvertDbReceive : public callback::ReceiveReport<target::ScriptResolvableReport>
{
  virtual void start( const Resolvable::constPtr & script_r,
                      const Pathname & path_r,
                      Task task_r )
  {
    SEC << __FUNCTION__ << endl
    << "  " << script_r << endl
    << "  " << path_r   << endl
    << "  " << task_r   << endl;
  }

  virtual bool progress( Notify notify_r, const std::string & text_r )
  {
    SEC << __FUNCTION__ << endl
    << "  " << notify_r << endl
    << "  " << text_r   << endl;
    return true;
  }

  virtual void problem( const std::string & description_r )
  {
    SEC << __FUNCTION__ << endl
    << "  " << description_r << endl;
  }

  virtual void finish()
  {
    SEC << __FUNCTION__ << endl;
  }

};

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
///////////////////////////////////////////////////////////////////

struct AddResolvables
{
  bool operator()( const Source_Ref & src ) const
  {
    getZYpp()->addResolvables( src.resolvables() );
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
  { return pi.status().setTransactValue( _newVal, _causer ); }
};

struct StatusReset : public SetTransactValue
{
  StatusReset()
  : SetTransactValue( ResStatus::KEEP_STATE, ResStatus::USER )
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
/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  //zypp::base::LogControl::instance().logfile( "log.restrict" );
  INT << "===[START]==========================================" << endl;

  ConvertDbReceive r;
  r.connect();

  ResPool pool( getZYpp()->pool() );

  if ( 1 )
    {
      zypp::base::LogControl::TmpLineWriter shutUp;
      getZYpp()->initTarget( sysRoot );
      USR << "Added target: " << pool << endl;
    }

  if ( 1 ) {
    //zypp::base::LogControl::TmpLineWriter shutUp;
    //SourceManager::sourceManager()->restore( sysRoot );
    if ( 1 || SourceManager::sourceManager()->allSources().empty() )
      {
        Source_Ref src1( createSource( "dir:/Local/script" ) );
        SourceManager::sourceManager()->addSource( src1 );
        //SourceManager::sourceManager()->store( sysRoot, true );
      }
    for_each( SourceManager::sourceManager()->Source_begin(), SourceManager::sourceManager()->Source_end(),
              AddResolvables() );
    dumpRange( USR << "Sources: ",
               SourceManager::sourceManager()->Source_begin(), SourceManager::sourceManager()->Source_end()
               ) << endl;
  }

  MIL << *SourceManager::sourceManager() << endl;
  MIL << pool << endl;
  //vdumpPoolStats( SEC, pool.begin(), pool.end() ) << endl;

  if ( 1 )
    {
#define selt(K,N) selectForTransact( nameKindProxy<K>( pool, #N ) )
      selt( Script, fetchmsttfonts.sh-patch-fetchmsttfonts.sh-2 );
#undef selt
    }

  vdumpPoolStats( USR << "Transacting:"<< endl,
                  make_filter_begin<resfilter::ByTransact>(pool),
                  make_filter_end<resfilter::ByTransact>(pool) ) << endl;

  if ( 1 ) {
    bool eres, rres;
    {
      //zypp::base::LogControl::TmpLineWriter shutUp;
      //zypp::base::LogControl::instance().logfile( "SOLVER" );
      eres = getZYpp()->resolver()->establishPool();
      rres = getZYpp()->resolver()->resolvePool();
    }
    MIL << "est " << eres << " slv " << rres << endl;
  }

  dumpPoolStats( USR << "Transacting:"<< endl,
                  make_filter_begin<resfilter::ByTransact>(pool),
                  make_filter_end<resfilter::ByTransact>(pool) ) << endl;


  if ( 1 )
    {
       ZYppCommitPolicy policy;
       policy.rpmNoSignature();
       ZYppCommitResult res( getZYpp()->commit( policy ) );
    }

  INT << "===[END]============================================" << endl << endl;
  zypp::base::LogControl::instance().logNothing();
  return 0;
}

