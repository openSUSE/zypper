#include "Tools.h"

#include <zypp/PoolQuery.h>

///////////////////////////////////////////////////////////////////

static const Pathname sysRoot( getenv("SYSROOT") ? getenv("SYSROOT") : "/Local/ROOT" );

///////////////////////////////////////////////////////////////////

bool solve()
{
  bool rres = false;
  {
    //zypp::base::LogControl::TmpLineWriter shutUp;
    //rres = test.resolver().resolvePool();
  }
  if ( ! rres )
  {
    ERR << "resolve " << rres << endl;
    return false;
  }
  MIL << "resolve " << rres << endl;
  return true;
}

int main( int argc, char * argv[] )
try {
  --argc;
  ++argv;
  zypp::base::LogControl::instance().logToStdErr();
  INT << "===[START]==========================================" << endl;
  ZConfig::instance();
  TestSetup::LoadSystemAt( sysRoot );
  ///////////////////////////////////////////////////////////////////
  ResPool   pool( ResPool::instance() );
  sat::Pool satpool( sat::Pool::instance() );
  ///////////////////////////////////////////////////////////////////
  dumpRange( USR, satpool.reposBegin(), satpool.reposEnd() ) << endl;
  USR << "pool: " << pool << endl;
  ///////////////////////////////////////////////////////////////////

  if ( 1 )
  {
    Measure x("-");
    sat::LookupAttr q( sat::SolvAttr::updateReference );
    for_( res, q.begin(), q.end() )
    {
      MIL << res << endl;
    }
  }

  if ( 0 )
  {
    Measure( "x" );

    PoolQuery q;
    q.setMatchSubstring();
    q.setCaseSensitive( false );
    q.addAttribute( sat::SolvAttr::updateReference );

    for_( it, q.begin(), q.end() )
    {
      // for each matching patch
      MIL << *it << endl;

      if ( 0 )
      {
        for_( d, it.matchesBegin(), it.matchesEnd() )
        {
          // for each matching updateReferenceId in that patch:
          DBG << " - " << d->inSolvAttr() << "\t\"" << d->asString() << "\" has type \""
              << d->subFind( sat::SolvAttr::updateReferenceType ).asString() << "\"" << endl;
          for_( s, d->subBegin(), d->subEnd() )
          {
            DBG << "    -" << s.inSolvAttr() << "\t\"" << s.asString() << "\"" << endl;
          }
        }
      }
    }
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

