#include <ctime>

#include <iostream>
#include <list>
#include <map>
#include <set>

#include "Measure.h"
#include "Printing.h"
#include "Tools.h"

#include <zypp/base/String.h>
#include <zypp/base/Exception.h>
#include <zypp/base/PtrTypes.h>
#include <zypp/base/Iterator.h>
#include <zypp/base/Algorithm.h>
#include <zypp/base/Functional.h>
#include <zypp/base/ProvideNumericId.h>
#include <zypp/base/ProvideNumericId.h>

#include "zypp/NVRAD.h"
#include "zypp/ResPool.h"
#include "zypp/ResFilters.h"
#include "zypp/CapFilters.h"
#include "zypp/Package.h"
#include "zypp/Language.h"
#include "zypp/NameKindProxy.h"

#include <zypp/SourceManager.h>
#include <zypp/SourceFactory.h>
#include <zypp/source/susetags/SuseTagsImpl.h>

#include "zypp/ZYppFactory.h"
#include "zypp/ResPoolProxy.h"
#include "zypp/ResPoolProxy.h"
#include "zypp/target/rpm/RpmDb.h"

using namespace std;
using namespace zypp;
using namespace zypp::ui;
using namespace zypp::functor;

///////////////////////////////////////////////////////////////////

static const Pathname sysRoot( "/Local/ROOT" );
static const Url      instSrc( "dir:/Local/SLES10" );
//static const Url      instSrc( "dir:/Local/FACTORY" );

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

///////////////////////////////////////////////////////////////////

template <class _Iterator>
  std::ostream & vdumpPoolStats( std::ostream & str,
                                 _Iterator begin_r, _Iterator end_r )
  {
    pool::PoolStats stats;
   // std::for_each( begin_r, end_r,
//
    //               functor::chain( setTrue_c(PrintPoolItem()),
    //                               setTrue_c(functor::functorRef<void,ResObject::constPtr>(stats)) )

     //            );
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

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  //zypp::base::LogControl::instance().logfile( "xxx" );
  INT << "===[START]==========================================" << endl;

  ResPool pool( getZYpp()->pool() );

  if ( 0 )
    {
      Measure x( "initTarget " + sysRoot.asString() );
      getZYpp()->initTarget( sysRoot );
      getZYpp()->addResolvables( getZYpp()->target()->resolvables(), true );
      INT << "Added target: " << pool << endl;
    }

  if ( 0 ) {
    SourceManager::sourceManager()->restore( sysRoot );
    if ( SourceManager::sourceManager()->allSources().empty() )
      {
        Source_Ref src( createSource( instSrc ) );
        SourceManager::sourceManager()->addSource( src );
        SourceManager::sourceManager()->store( sysRoot, true );
      }

    Source_Ref src( *SourceManager::sourceManager()->Source_begin() );
    getZYpp()->addResolvables( src.resolvables() );
    INT << "Added source: " << pool << endl;
  }

  Source_Ref src1( createSource( "dir:/Local/SUSE-Linux-10.1-Build_830-Addon-BiArch/CD1" ) );
  Source_Ref src2( createSource( "dir:/Local/SUSE-Linux-10.1-Build_830-i386/CD1" ) );
  INT << "Pool: " << pool << endl;
  getZYpp()->addResolvables( src1.resolvables() );
  INT << "Added source1: " << pool << endl;
  getZYpp()->addResolvables( src2.resolvables() );
  INT << "Added source2: " << pool << endl;




#if 0
  Source_Ref src( *SourceManager::sourceManager()->Source_begin() );
  const std::list<Pathname> srcKeys( src.publicKeys() );
  MIL << src << endl;
  DBG << srcKeys << endl;

  target::rpm::RpmDb rpm;
  rpm.initDatabase( sysRoot );
  std::set<Edition> rpmKeys( rpm.pubkeys() );
  MIL << rpm << endl;
  DBG << rpmKeys << endl;

  ResPool pool( getZYpp()->pool() );
  getZYpp()->addResolvables( src.resolvables() );
  SEC << pool << endl;

  rpm.closeDatabase();
#endif

  INT << "===[END]============================================" << endl << endl;
  return 0;
}

