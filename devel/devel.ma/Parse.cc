#include <ctime>

#include <iostream>
#include <list>
#include <map>
#include <set>

#include "Measure.h"
#include "Printing.h"

#include <zypp/base/Logger.h>
#include <zypp/base/LogControl.h>
#include <zypp/base/String.h>
#include <zypp/base/Exception.h>
#include <zypp/base/PtrTypes.h>
#include <zypp/base/Iterator.h>
#include <zypp/base/Algorithm.h>
#include <zypp/base/Functional.h>
#include <zypp/base/ProvideNumericId.h>

#include "zypp/NVRAD.h"
#include "zypp/ResPool.h"
#include "zypp/ResFilters.h"
#include "zypp/CapFilters.h"

#include <zypp/SourceFactory.h>
#include <zypp/source/susetags/SuseTagsImpl.h>

#include "zypp/ResPoolManager.h"
#include "zypp/ResPoolProxy.h"

using namespace std;
using namespace zypp;
using namespace zypp::ui;
using namespace zypp::functor;

///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////

template<class _IntT>
  struct Counter
  {
    Counter()
    : _value( _IntT(0) )
    {}

    Counter( _IntT value_r )
    : _value( _IntT( value_r ) )
    {}


    operator _IntT &()
    { return _value; }

    operator const _IntT &() const
    { return _value; }

    _IntT _value;
  };

struct Rstats : public std::unary_function<ResObject::constPtr, void>
{
  void operator()( ResObject::constPtr ptr )
  {
    ++_total;
    ++_perKind[ptr->kind()];
  }

  typedef std::map<ResolvableTraits::KindType,Counter<unsigned> > KindMap;
  Counter<unsigned> _total;
  KindMap           _perKind;
};

std::ostream & operator<<( std::ostream & str, const Rstats & obj )
{
  str << "Total: " << obj._total;
  for( Rstats::KindMap::const_iterator it = obj._perKind.begin(); it != obj._perKind.end(); ++it )
    {
      str << endl << "  " << it->first << ":\t" << it->second;
    }
  return str;
}

template<class _Iterator>
  void rstats( _Iterator begin, _Iterator end )
  {
    DBG << __PRETTY_FUNCTION__ << endl;
    Rstats stats;
    for_each( begin, end, functorRef<void,ResObject::constPtr>(stats) );
    MIL << stats << endl;
  }

///////////////////////////////////////////////////////////////////

struct XByInstalled : public std::unary_function<ui::Selectable::constPtr,bool>
{
  bool operator()( const ui::Selectable::constPtr & obj ) const
  {
    return obj->hasInstalledObj();
  }
};

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace resfilter
  { /////////////////////////////////////////////////////////////////
    /** Select ResObject by kind. */
    struct Mtest : public PoolItemFilterFunctor
    {
      bool operator()( const PoolItem & p ) const
      {
        p.status().setTransact(true, ResStatus::USER );
        return true;
      }
    };

    /////////////////////////////////////////////////////////////////
  } // namespace resfilter
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

struct X : public base::ProvideNumericId<X>
{

};

template<class _Who>
  void who( _Who & w )
  {
    INT << __PRETTY_FUNCTION__ << endl;
  }

ostream & operator<<( ostream & str, const X & obj )
{
  return str << "ID(" << obj.numericId() << ")";
}

#include "zypp/detail/ImplConnect.h"
#include "zypp/detail/ResObjectImplIf.h"
#include "zypp/Package.h"
struct ImplTest
{
  void operator()( const PoolItem & pi )
  {
    who( detail::ImplConnect::resimpl( *pi.resolvable() ) );
    Package::constPtr p( dynamic_pointer_cast<const Package>(pi.resolvable()) );
    if ( p )
      who( detail::ImplConnect::resimpl( *p ) );
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

  string infile( "p" );
  if (argc >= 2 )
    infile = argv[1];

  Url url("dir:/Local/ma/zypp/libzypp/devel/devel.ma/CD1");
  Measure x( "SourceFactory.create" );
  Source_Ref src( SourceFactory().createFrom( url ) );
  x.stop();
  Source_Ref trg( SourceFactory().createFrom( url ) );

  //Source_Ref src( SourceFactory().createFrom( new source::susetags::SuseTagsImpl(infile) ) );
  //MIL << src.resolvables().size() << endl;

  ResPoolManager pool;
  x.start( "pool.insert" );
  ResStore::const_iterator last = src.resolvables().begin();
  std::advance( last, 5 );
  pool.insert( src.resolvables().begin(), last );
  x.stop();
  MIL << pool << endl;

  ResPool query( pool.accessor() );
  rstats( query.begin(), query.end() );

  ResPoolProxy y2pm( query );
  y2pm.saveState<Package>();
  //pool.insert( trg.resolvables().begin(), trg.resolvables().end(), true );
  y2pm = ResPoolProxy( query );
  std::for_each( query.begin(), query.end(), Print<PoolItem>() );
  std::for_each( query.begin(), query.end(), resfilter::Mtest() );
  y2pm.restoreState<Package>();
  std::for_each( query.begin(), query.end(), Print<PoolItem>() );
  std::for_each( query.begin(), query.end(), ImplTest() );


  INT << "===[END]============================================" << endl << endl;
  return 0;
}

