#include "Tools.h"

#include <zypp/PoolQuery.h>
#include <zypp/target/rpm/librpmDb.h>
#include <zypp/parser/ProductFileReader.h>

#include "zypp/sat/WhatObsoletes.h"
#include "zypp/ExternalProgram.h"

///////////////////////////////////////////////////////////////////

//static const Pathname sysRoot( getenv("SYSROOT") ? getenv("SYSROOT") : "/Local/ROOT" );
//static const Pathname sysRoot( "/tmp/ToolScanRepos" );
static const Pathname sysRoot( "/" );

///////////////////////////////////////////////////////////////////

bool solve()
{
  bool rres = false;
  {
    //zypp::base::LogControl::TmpLineWriter shutUp;
    //getZYpp()->resolver()->setOnlyRequires( true );
    rres = getZYpp()->resolver()->resolvePool();
  }
  if ( ! rres )
  {
    ERR << "resolve " << rres << endl;
    getZYpp()->resolver()->problems();
    return false;
  }
  MIL << "resolve " << rres << endl;
  return true;
}

bool upgrade()
{
  bool rres = false;
  {
    //zypp::base::LogControl::TmpLineWriter shutUp;
    Measure x( "Upgrade" );
    rres = getZYpp()->resolver()->doUpgrade();
  }
  if ( ! rres )
  {
    Measure x( "Upgrade Error" );
    ERR << "upgrade " << rres << endl;
    getZYpp()->resolver()->problems();
    return false;
  }
  MIL << "upgrade " << rres << endl;
  return true;
}

namespace zypp
{
  namespace target
  {
    void writeUpgradeTestcase();
  }
}

std::ostream & operator<<( std::ostream & str, const sat::Solvable::SplitIdent & obj )
{
  str << "{" << obj.ident() << "}{" << obj.kind() << "}{" << obj.name () << "}" << endl;
  return str;
}

namespace zypp {
std::ostream & dumpOn( std::ostream & str, const Url & obj )
{
  str << "{" << obj.getHost() << "}{" << obj.getPort() << "}";
  return str;
}
}

int main( int argc, char * argv[] )
try {
  --argc,++argv;
  zypp::base::LogControl::instance().logToStdErr();
  INT << "===[START]==========================================" << endl;
  ///////////////////////////////////////////////////////////////////
  if ( sysRoot == "/" )
    ::unsetenv( "ZYPP_CONF" );
  ResPool   pool( ResPool::instance() );
  sat::Pool satpool( sat::Pool::instance() );
  ///////////////////////////////////////////////////////////////////
  dumpRange( WAR << "satpool.multiversion " , satpool.multiversionBegin(), satpool.multiversionEnd() ) << endl;
  TestSetup::LoadSystemAt( sysRoot, Arch_i586 );
  ///////////////////////////////////////////////////////////////////

  char * fix[] = {
      "test"
  };
  argv = fix;
  argc = arraySize(fix);
  for ( ; argc; --argc,++argv )
  {
    ui::Selectable::Ptr p( getSel<Package>( *argv ) );
    if ( p )
      USR << p->setToDelete() << endl;
    else
      ERR << p << endl;
  }

  std::set<PoolItem> todel;
  {
    getZYpp()->resolver()->setCleandepsOnRemove( false );
    SEC << "=== Solve noclean:" << endl;
    solve();
    std::copy( make_filter_begin<resfilter::ByTransact>(pool),
	       make_filter_end<resfilter::ByTransact>(pool),
	       std::inserter( todel, todel.begin() ) );
    WAR << todel << endl;
  }
  {
    getZYpp()->resolver()->setCleandepsOnRemove( true );
    SEC << "=== Solve clean:" << endl;
    solve();
    SEC << "========================================================" << endl;
    for_( it, make_filter_begin<resfilter::ByTransact>(pool), make_filter_end<resfilter::ByTransact>(pool) )
    {
      ( todel.find( *it ) == todel.end() ? INT : USR ) << *it << endl;
    }
    SEC << "========================================================" << endl;
  }



  ///////////////////////////////////////////////////////////////////
  INT << "===[END]============================================" << endl << endl;
  zypp::base::LogControl::instance().logNothing();
  return 0;
}
catch ( const Exception & exp )
{
  INT << exp << endl << exp.historyAsString();
}
catch (...)
{}

