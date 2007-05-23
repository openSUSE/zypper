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
#include "zypp/Pattern.h"
#include "zypp/Language.h"
#include "zypp/PackageKeyword.h"
#include "zypp/NameKindProxy.h"
#include "zypp/pool/GetResolvablesToInsDel.h"

#include "zypp/parser/tagfile/TagFileParser.h"
#include "zypp/parser/TagParser.h"
#include "zypp/parser/susetags/PackagesFileReader.h"
#include "zypp/parser/susetags/PackagesLangFileReader.h"
#include "zypp/parser/susetags/PatternFileReader.h"
#include "zypp/parser/susetags/ContentFileReader.h"
#include "zypp/parser/susetags/RepoIndex.h"

#include "zypp2/parser/susetags/RepoParser.h"

using namespace std;
using namespace zypp;
using namespace zypp::functor;

using zypp::parser::tagfile::TagFileParser;
using zypp::parser::TagParser;

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

struct MediaChangeReceive : public callback::ReceiveReport<media::MediaChangeReport>
{
  virtual Action requestMedia( Source_Ref source
                               , unsigned mediumNr
                               , Error error
                               , const std::string & description )
  {
    SEC << __FUNCTION__ << endl
    << "  " << source << endl
    << "  " << mediumNr << endl
    << "  " << error << endl
    << "  " << description << endl;
    return IGNORE;
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

struct AddResolvables
{
  bool operator()( const Source_Ref & src ) const
  {
    getZYpp()->addResolvables( src.resolvables() );
    return true;
  }
};

///////////////////////////////////////////////////////////////////


std::ostream & operator<<( std::ostream & str, const iostr::EachLine & obj )
{
  str << "(" << obj.valid() << ")[" << obj.lineNo() << "|" << obj.lineStart() << "]{" << *obj << "}";
  return str;

}

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

using namespace zypp::parser::susetags;
#include "zypp2/cache/CacheStore.h"

void consumeIndex( const parser::susetags::RepoIndex_Ptr & index_r )
{
  SEC << "[Index]" << index_r << endl;
}

void consumeProd( const data::Product_Ptr & prod_r )
{
  SEC << "[Prod]" << prod_r << endl;
}

void consumePkg( const data::Package_Ptr & pkg_r )
{
  //MIL << "[Pkg]" << pkg_r << endl;
}

void consumeSrcPkg( const data::SrcPackage_Ptr & pkg_r )
{
  //DBG << "[Src]" << pkg_r << endl;
}

void consumePat( const data::Pattern_Ptr & pat_r )
{
  MIL << "[Pat]" << pat_r << endl;
}

void pPackages( const Pathname & p )
{
  Measure x( p.basename() );
  PackagesFileReader tp;
  tp.setPkgConsumer( consumePkg );
  tp.setSrcPkgConsumer( consumeSrcPkg );
  tp.parse( p );
}

void pPackagesLang( const Pathname & p, const Locale & locale_r )
{
  Measure x( p.basename() );
  PackagesLangFileReader tp;
  tp.setLocale( locale_r );
  tp.setPkgConsumer( consumePkg );
  tp.setSrcPkgConsumer( consumeSrcPkg );
  tp.parse( p );
}

void pPattern( const Pathname & p )
{
  Measure x( p.basename() );
  PatternFileReader tp;
  tp.setConsumer( consumePat );
  tp.parse( p );
}
 /******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  //zypp::base::LogControl::instance().logfile( "log.restrict" );
  INT << "===[START]==========================================" << endl;

  Pathname dbdir( "store" );
  Pathname reporoot( "lmd" );

  cache::CacheStore store( dbdir );
  data::RecordId catalogId = store.lookupOrAppendCatalog( Url("dir:///"), "/" );
  {
    Measure x( "XXXXXXXXXXXXX" );

    parser::susetags::RepoParser repo( catalogId, store );
    repo.parse( reporoot );

    store.commit();
  }


#if 0
  ContentFileReader tp;
  tp.setProductConsumer( consumeProd );
  tp.setRepoIndexConsumer( consumeIndex );
  //tp.setSrcPkgConsumer( consumeSrcPkg );
  tp.parse( p );


  //try
  {
    //Pathname dbdir( "/Local/ma/zypp-TRUNK/BUILD/libzypp/devel/devel.ma/store" );


    Pathname dbdir( "store" );
    Pathname metadir( "lmd" );

    cache::CacheStore store( dbdir );
    data::RecordId catalogId = store.lookupOrAppendCatalog( Url("http://www.google.com"), "/" );

    RepoParser( metadir, catalogId, store );

  }

    try
    {
      ZYpp::Ptr z = getZYpp();

      Pathname dbfile( "data.db" );
      cache::CacheStore store(getenv("PWD"));

      data::RecordId catalog_id = store.lookupOrAppendCatalog( Url("http://www.google.com"), "/");

      PackagesParser parser( catalog_id, store);
      Measure m;
      parser.start(argv[1], &progress_function);
      m.elapsed();
    }
    catch ( const Exception &e )
    {
      cout << "ups! " << e.msg() << std::endl;
    }
#endif

  INT << "===[END]============================================" << endl << endl;
  zypp::base::LogControl::instance().logNothing();
  return 0;

  Pathname proot( "lmd/suse/setup/descr" );

  pPackages( proot/"packages" );
  //pPackages( proot/"packages.gz" );
  pPackagesLang( proot/"packages.de", Locale("de") );
  //pPackagesLang( proot/"packages.de.gz", Locale("de") );
  pPattern( proot/"base-10.3-30.x86_64.pat" );

  if ( 0 )
  {
    Measure x( "lmd.idx" );
    std::ifstream fIndex( "lmd.idx" );
    for( iostr::EachLine in( fIndex ); in; in.next() )
    {
      Measure x( *in );
      std::ifstream fIn( (*in).c_str() );
      for( iostr::EachLine l( fIn ); l; l.next() )
      {
	;
      }
    }
  }
  INT << "===[END]============================================" << endl << endl;
  zypp::base::LogControl::instance().logNothing();
  return 0;
}

