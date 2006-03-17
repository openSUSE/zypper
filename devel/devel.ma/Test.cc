#include <iostream>

#include "zypp/base/LogControl.h"

#include <zypp/Package.h>
#include <zypp/detail/PackageImpl.h>
#include <zypp/CapFactory.h>
#include <zypp/CapSet.h>

#include "zypp/ZYppFactory.h"
#include "zypp/SystemResObject.h"
#include "zypp/ResPool.h"

#include "Tools.h"

using namespace std;
using namespace zypp;

template<class _LIter, class _RIter, class _BinaryFunction>
  inline _BinaryFunction
  nest_for_earch( _LIter lbegin, _LIter lend,
                  _RIter rbegin, _RIter rend,
                  _BinaryFunction fnc )
  {
    for ( ; lbegin != lend; ++lbegin )
      for ( _RIter r = rbegin; r != rend; ++r )
        fnc( *lbegin, *r );
    return fnc;
  }

template<class _Iter, class _BinaryFunction>
  inline _BinaryFunction
  nest_for_earch( _Iter begin, _Iter end,
                  _BinaryFunction fnc )
  { return nest_for_earch( begin, end, begin, end, fnc ); }

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////


  void matches( const Capability & lhs, const Capability & rhs )
  {
    SEC << "matches " << lhs << " <=> " << rhs << endl;
    SEC << "        " << lhs.matches(rhs) << endl;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////



template<typename _Res>
  struct CapSetInsert : public std::unary_function<const std::string &, void>
  {
    CapSet &   _x;
    CapFactory _f;
    CapSetInsert( CapSet & x )
    : _x(x)
    {}
    void operator()( const std::string & v )
    { _x.insert( _f.parse( ResTraits<_Res>::kind, v ) ); }
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

  ResStore inst;
  ResStore avail;

  // Collect basic Resolvable data
  NVRAD dataCollect;

  dataCollect.name    = "foo";
  dataCollect.edition = Edition("1.0","42");
  dataCollect.arch    = Arch_i386;

  const char * depstrings[] = {
    "hal()",
    "modalias()",
    "hal(foo)",
    "modalias(bab)",
  };

  for_each( depstrings, ( depstrings + ( sizeof(depstrings) / sizeof(const char *) ) ),
            CapSetInsert<Package>(dataCollect[Dep::PROVIDES]) );


  //nest_for_earch( dataCollect[Dep::PROVIDES].begin(), dataCollect[Dep::PROVIDES].end(),
  //                &matches );

  // create the Package object
  detail::ResImplTraits<detail::PackageImpl>::Ptr pkgImpl;
  Package::Ptr pkg( detail::makeResolvableAndImpl( dataCollect, pkgImpl ) );
  pkg = detail::makeResolvableAndImpl( dataCollect, pkgImpl );
  DBG << *pkg << endl;
  DBG << pkg->deps() << endl;
  avail.insert( pkg );

  // create the System object
  SystemResObject::Ptr sys = SystemResObject::instance();
  DBG << *sys << endl;
  DBG << sys->deps() << endl;
  //inst.insert( sys );
  avail.insert( pkg );

  // feed pool
  getZYpp()->addResolvables( avail );
  getZYpp()->addResolvables( inst, true );

  // print stats
  ResPool query( getZYpp()->pool() );
  rstats( query.begin(), query.end() );

  // select system resolvable for transact
  query.byKindBegin<SystemResObject>()->status().setTransact( true, ResStatus::USER );
  std::for_each( query.begin(), query.end(), Print<PoolItem>() );

  SEC << getZYpp()->resolver()->establishPool() << endl;
  std::for_each( query.begin(), query.end(), Print<PoolItem>() );

  SEC << getZYpp()->resolver()->resolvePool() << endl;
  std::for_each( query.begin(), query.end(), Print<PoolItem>() );

  INT << "===[END]============================================" << endl;
  return 0;
}
