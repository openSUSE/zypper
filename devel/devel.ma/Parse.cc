#include <ctime>

#include <iostream>
#include <list>
#include <map>
#include <set>

#include "Measure.h"
#include "Printing.h"
#include "Tools.h"

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

template<>
  struct PrintPtr<ui::Selectable::Ptr> : public std::unary_function<ui::Selectable::Ptr, bool>
  {
    bool operator()( const ui::Selectable::Ptr & obj )
    {
      if ( obj ) {
        USR << *obj << std::endl;
        std::for_each( obj->availableBegin(), obj->availableEnd(),
                       PrintPtr<ResObject::constPtr>() );
      }
      else
        USR << "(NULL)" << std::endl;
      return true;
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

  Source_Ref src( createSource("dir:/Local/ma/zypp/libzypp/devel/devel.ma/SOURCE") );
  Source_Ref trg( createSource("dir:/Local/ma/zypp/libzypp/devel/devel.ma/TARGET") );

  ResPoolManager pool;
  pool.insert( src.resolvables().begin(), src.resolvables().end() );
  pool.insert( trg.resolvables().begin(), trg.resolvables().end(), true );

  ResPool query( pool.accessor() );
  rstats( query.begin(), query.end() );
  std::for_each( query.begin(), query.end(), Print<PoolItem>() );

  ResPoolProxy y2pm( query );
  std::for_each( y2pm.byKindBegin<Package>(), y2pm.byKindEnd<Package>(),
                 PrintPtr<ui::Selectable::Ptr>() );


  INT << "===[END]============================================" << endl << endl;
  return 0;
}

