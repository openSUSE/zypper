#include <iostream>
#include "Tools.h"
#include <boost/bind/protect.hpp>

#include <zypp/base/LogControl.h>
#include <zypp/base/LogTools.h>
#include <zypp/base/IOStream.h>
#include <zypp/base/Function.h>
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
#include <zypp/base/InputStream.h>
#include <zypp/base/DefaultIntegral.h>


#include <zypp/ui/PatternExpander.h>
#include <zypp/ui/PatternContents.h>


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
      DBG << __PRETTY_FUNCTION__ << o << endl;
      return true;
    }
};

struct Select
{
  template<class _Tp>
    bool operator()( const _Tp & o )
    {
      Pattern::constPtr p( asKind<Pattern>(o) );
      if ( ! ( p->includes().empty() && p->extends().empty() ) )
        {
          MIL << p << endl;
          dumpRange( DBG << "Includes: ", p->includes().begin(), p->includes().end() ) << endl;
          dumpRange( DBG << "Extends: ", p->extends().begin(), p->extends().end() ) << endl;
        }
      return true;
    }
};



struct ExpandPattern
{
  ExpandPattern( ResPool pool_r )
  : _pool( pool_r )
  {}

  bool operator()( ResObject::constPtr obj_r ) const
  {
    PatternContents a( asKind<Pattern>(obj_r) );
    a.install_packages();
    return true;
  }

  bool operatorx( ResObject::constPtr obj_r ) const
  {
    PatternExpander a( _pool );
    MIL << a.expand( obj_r ) << " for " << obj_r << endl;
    for_each( a.begin(), a.end(), Dump() );
    return true;
  }


  ResPool _pool;
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

  if ( 0 )
    {
      zypp::base::LogControl::TmpLineWriter shutUp;

      getZYpp()->initializeTarget( sysRoot );
      getZYpp()->addResolvables( getZYpp()->target()->resolvables(), true );
    }

  Source_Ref src1;
  if ( 1 )
    {
      zypp::base::LogControl::TmpLineWriter shutUp;

      //src1 = createSource( "dir:/Local/SLES10" );
      src1 = createSource( "dir:/Local/openSUSE-10.2-Alpha5test-x86_64/CD1" );
      getZYpp()->addResolvables( src1.resolvables() );

    }

  for_each( pool.byKindBegin<Pattern>(),
            pool.byKindEnd<Pattern>(),
            ExpandPattern(pool) );

#if 0
  ResPoolProxy ui( getZYpp()->poolProxy() );
  Select sel = for_each( ui.byKindBegin<Pattern>(),
                         ui.byKindEnd<Pattern>(),
                         Select() );
#endif



  INT << "===[END]============================================" << endl << endl;
  new zypp::base::LogControl::TmpLineWriter;
  return 0;
}

