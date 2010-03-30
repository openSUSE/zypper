#include "Tools.h"

#include <zypp/PoolQuery.h>
#include <zypp/target/rpm/librpmDb.h>
#include <zypp/parser/ProductFileReader.h>
#include "zypp/pool/GetResolvablesToInsDel.h"
#include "zypp/sat/WhatObsoletes.h"

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

int main( int argc, char * argv[] )
try {
  --argc;
  ++argv;
  zypp::base::LogControl::instance().logToStdErr();
  INT << "===[START]==========================================" << endl;
  ///////////////////////////////////////////////////////////////////
  if ( sysRoot == "/" )
    ::unsetenv( "ZYPP_CONF" );
  ZConfig::instance().setTextLocale( Locale("de_DE") );
  ResPool   pool( ResPool::instance() );
  sat::Pool satpool( sat::Pool::instance() );
  ///////////////////////////////////////////////////////////////////
  dumpRange( WAR << "satpool.multiversion " , satpool.multiversionBegin(), satpool.multiversionEnd() ) << endl;
  TestSetup::LoadSystemAt( sysRoot, Arch_i586 );
  ///////////////////////////////////////////////////////////////////

  ui::Selectable::Ptr s( getSel<Package>( "libzypp" ) );
  MIL << s << endl;
  DBG << s->setStatus( ui::S_Taboo ) << endl;
  DBG << s->setStatus( ui::S_Protected ) << endl;
  MIL << s << endl;
  DBG << s->setStatus( ui::S_Update ) << endl;
  MIL << s << endl;


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

