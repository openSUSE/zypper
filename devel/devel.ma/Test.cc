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
#include <zypp/base/Debug.h>

#include "zypp/NVRAD.h"
#include "zypp/ResPool.h"
#include "zypp/ResFilters.h"
#include "zypp/CapFilters.h"
#include "zypp/Package.h"

#include <zypp/SourceManager.h>
#include <zypp/SourceFactory.h>
#include <zypp/source/susetags/SuseTagsImpl.h>

#include "zypp/ZYppFactory.h"
#include "zypp/ResPoolProxy.h"
#include "zypp/ResPoolProxy.h"
#include <zypp/target/rpm/RpmDb.h>

using namespace std;
using namespace zypp;
using namespace zypp::ui;
using namespace zypp::functor;
using namespace zypp::debug;

///////////////////////////////////////////////////////////////////

static const Pathname sysRoot( "/Local/ROOT" );
static const Url      instSrc( "dir:/Local/SLES10" );
//static const Url      instSrc( "dir:/Local/FACTORY" );

///////////////////////////////////////////////////////////////////

template<class _Tp>
  ostream & operator<<( ostream & str, const set<_Tp> & obj )
  {
    str << "Size(" << obj.size() << ") {";
    std::for_each( obj.begin(), obj.end(), PrintOn<_Tp>(str,"  ",true) );
    return str << endl << "}";
  }

template<class _Tp>
  ostream & operator<<( ostream & str, const list<_Tp> & obj )
  {
    str << "Size(" << obj.size() << ") {";
    std::for_each( obj.begin(), obj.end(), PrintOn<_Tp>(str,"  ",true) );
    return str << endl << "}";
  }

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

  if ( 0 ) {
    Measure x( "initTarget " + sysRoot.asString() );
    getZYpp()->initTarget( sysRoot );
  }

  SourceManager::sourceManager()->restore( sysRoot );
  if ( 0 && SourceManager::sourceManager()->allSources().empty() )
    {
      Source_Ref src( createSource( instSrc ) );
      SourceManager::sourceManager()->addSource( src );
      //SourceManager::sourceManager()->store( sysRoot, true );
    }

  target::rpm::RpmDb rpm;
  rpm.initDatabase( sysRoot );
  std::set<Edition> pubkeys( rpm.pubkeys() );
  USR << "RpmDb::pubkeys " << pubkeys << endl;

  Source_Ref src( createSource( instSrc ) );
  std::list<Pathname> publicKeys( src.publicKeys() );
  USR << "Source::pubkeys " << publicKeys << endl;

  ResPool pool( getZYpp()->pool() );
  getZYpp()->addResolvables( src.resolvables() );

  rstats( pool.begin(), pool.end() );
  //std::for_each( query.begin(), query.end(), Print<PoolItem>() );


  rpm.closeDatabase();

  INT << "===[END]============================================" << endl << endl;
  return 0;
}

