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

struct XByInstalled : public std::unary_function<ui::Selectable::constPtr,bool>
{
  bool operator()( const ui::Selectable::constPtr & obj ) const
  {
    return obj->hasInstalledObj();
  }
};

template<>
  struct PrintPtr<ui::Selectable::Ptr> : public std::unary_function<ui::Selectable::Ptr, bool>
  {
    bool operator()( const ui::Selectable::Ptr & obj )
    {
      if ( obj ) {
        MIL << obj->modifiedBy() << " " << obj->hasLicenceConfirmed() << endl;
        obj->set_status( ui::S_Install );
        obj->setLicenceConfirmed( true );
        MIL << "a " << obj->modifiedBy() << " " << obj->hasLicenceConfirmed() << endl;
        obj->set_status( ui::S_Del );
        obj->setLicenceConfirmed( false );
        MIL << "b " << obj->modifiedBy() << " " << obj->hasLicenceConfirmed() << endl;

#if 0
        USR << *obj << std::endl;
        std::for_each( obj->availableBegin(), obj->availableEnd(),
                       PrintPtr<ResObject::constPtr>() );
        if ( obj->availableBegin() != obj->availableEnd() )
          SEC << PrintPtr<ResObject::constPtr>()(*obj->availableBegin() )
              << " " << asKind<Package>(*obj->availableBegin())->vendor() << endl;
#endif
      }
      else
        USR << "(NULL)" << std::endl;
      return true;
    }
  };

struct Mpool
{
  bool operator()( const PoolItem & pi )
  {
    return pi.status().isLocked();
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

  ResPool pool( getZYpp()->pool() );

  if ( 1 )
    {
      Measure x( "initTarget " + sysRoot.asString() );
      getZYpp()->initTarget( sysRoot );
      getZYpp()->addResolvables( getZYpp()->target()->resolvables(), true );
      INT << "Added target: " << pool << endl;
    }

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

  NameKindProxy s( nameKindProxy<Selection>( pool, "default" ) );
  MIL << s << endl;


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

