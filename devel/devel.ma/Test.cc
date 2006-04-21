#include <ctime>
#include <iostream>
#include <fstream>
#include "Tools.h"

#include <zypp/base/IOStream.h>
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


using namespace std;
using namespace zypp;
using namespace zypp::ui;
using namespace zypp::functor;

///////////////////////////////////////////////////////////////////

static const Pathname sysRoot( "/Local/ROOT" );

///////////////////////////////////////////////////////////////////

struct DebugStats
{
  bool operator()( const ResObject::constPtr & ptr )
  {
    Source_Ref::NumericId srcid = ptr->source().numericId();
    unsigned mediaid = mediaId( ptr )
    USR << "S"  << srcid
        << "/M" << mediaid
        << " "  << ptr
        << endl;
    return true;
  }

  unsigned mediaId( const ResObject::constPtr ptr & ptr ) const
  {
    Package::constPtr pkg( asKind<Package>(ptr) );
    return pkg ? pkg->mediaId() : 0;
  }


  public:
    typedef std::map<ResolvableTraits::KindType,Counter<unsigned> > KindMap;
    Counter<unsigned> _total;
    KindMap           _perKind;
};


template <class _Iterator>
  std::ostream & vdumpPoolStats( std::ostream & str,
                                 _Iterator begin_r, _Iterator end_r )
  {
    DebugStats stats;
    std::for_each( begin_r, end_r,

                   functor::chain( setTrue_c(PrintPoolItem()),
                                   setTrue_c(functor::functorRef<void,ResObject::constPtr>(stats)) )

                 );
    return str << stats;
  }

///////////////////////////////////////////////////////////////////

inline bool selectForTransact( const NameKindProxy & nkp )
{
  if ( nkp.availableEmpty() )
    return false;

  return nkp.availableBegin()->status().setTransact( true, ResStatus::USER );
}

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
      getZYpp()->initTarget( sysRoot );
      getZYpp()->addResolvables( getZYpp()->target()->resolvables(), true );
      INT << "Added target: " << pool << endl;
    }

  if ( 0 )
    {
      Source_Ref src1( createSource( "dir:/mounts/machcd2/CDs/SLES-10-CD-i386-Beta10/CD1" ) );
      Source_Ref src2( createSource( "dir:/mounts/machcd2/kukuk/sles10-sp-i386/CD1" ) );
      getZYpp()->addResolvables( src1.resolvables() );
      getZYpp()->addResolvables( src2.resolvables() );
      INT << "Pool: " << pool << endl;
    }

  selectForTransact( nameKindProxy<Pattern>( pool, "default" ) );
  selectForTransact( nameKindProxy<Pattern>( pool, "x11" ) );
  selectForTransact( nameKindProxy<Pattern>( pool, "kde" ) );
  selectForTransact( nameKindProxy<Pattern>( pool, "OOo" ) );

  vdumpPoolStats( INT << "Transacting: ",
                  make_filter_begin<resfilter::ByTransact>(pool),
                  make_filter_end<resfilter::ByTransact>(pool) ) << endl;

  INT << "===[END]============================================" << endl << endl;
  return 0;
}
