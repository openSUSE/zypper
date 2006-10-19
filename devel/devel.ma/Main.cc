#include <iostream>
#include "Tools.h"

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

struct CapAndItemMatches
{
  CapAndItemMatches( const Capability & lhs_r )
  : _lhs( lhs_r )
  {}

  bool operator()( const CapAndItem & capitem_r ) const
  {
    return( _lhs.matches( capitem_r.cap ) == CapMatch::yes );
  }

  const Capability & _lhs;
};

inline int forEachMatchIn( const ResPool & pool_r, const Dep & dep_r, const Capability & lhs_r,
                           function<bool(const CapAndItem &)> action_r )
{
  std::string index( lhs_r.index() );
  return invokeOnEach( pool_r.byCapabilityIndexBegin( index, dep_r ),
                       pool_r.byCapabilityIndexEnd( index, dep_r ),
                       CapAndItemMatches( lhs_r ), // filter
                       action_r );
}


struct PatternExtendedInstallPackages
{
  PatternExtendedInstallPackages( ResPool pool_r )
  : _pool( pool_r )
  {}

  bool operator()( ResObject::constPtr obj_r ) const
  { return operator()( asKind<Pattern>(obj_r) ); }

  bool operator()( Pattern::constPtr pat_r ) const
  {
    if ( ! pat_r )
      return true;

    MIL << pat_r << endl;
    expandIncludes( pat_r );
    //selectIncludes( pat_r );
    return true;
  }

  void expandIncludes( const Pattern::constPtr & pat_r ) const
  {
    if ( ! pat_r->includes().empty() )
      {
        for_each( pat_r->includes().begin(),
                  pat_r->includes().end(),
                  bind( &PatternExtendedInstallPackages::expandInclude, this, _1 ) );
      }
  }

  void expandInclude( const Capability & include_r ) const
  {
    DBG << include_r << endl;
    forEachMatchIn( _pool, Dep::PROVIDES, include_r,
                    bind( &PatternExtendedInstallPackages::storeIncludeMatch, this, _1 ) );
  }

  bool storeIncludeMatch( const CapAndItem & capitem_r ) const
  {
    Dump()( capitem_r.item );
    return true;
  }


  private:
    ResPool _pool;
};

#define Default DefaultIntegral
typedef Default<unsigned,0> xCounter;

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  //zypp::base::LogControl::instance().logfile( "" );
  INT << "===[START]==========================================" << endl;

  xCounter x;
  DBG  << x << endl;
  Default<bool,true> a;
  Default<int,13> i;
  DBG  << i << endl;
  i = 15;
  DBG  << i << endl;
  i -= 3;
  DBG  << i << endl;
  Default<int,6> j;
  DBG  << (i*j) << endl;
  DBG  << Default<bool,true>() << endl;

  return 0;
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

      src1 = createSource( "dir:/Local/openSUSE-10.2-Alpha5test-x86_64/CD1" );
      getZYpp()->addResolvables( src1.resolvables() );

    }

  for_each( pool.byKindBegin<Pattern>(),
            pool.byKindEnd<Pattern>(),
            PatternExtendedInstallPackages( pool ) );




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

