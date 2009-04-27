#include "Tools.h"
#include <zypp/ResObjects.h>

#include <zypp/sat/LookupAttr.h>
#include <zypp/PoolQuery.h>
#include <zypp/sat/AttrMatcher.h>

static TestSetup test( Arch_x86_64 );  // use x86_64 as system arch

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;
  test.loadRepo( "/Local/ROOT/cache/solv/@System/solv" );

  Match t( Match::REGEX );

  MIL << (t|Match::STRING) << endl;
  MIL << (t|Match::NOCASE) << endl;
  MIL << (Match::STRING|t) << endl;
  MIL << (Match::STRING|Match::NOCASE) << endl;


  Match m = Match::STRING | Match::NOCASE | Match::GLOB ;
  m = Match::NOCASE | Match::STRING;
  MIL << m << endl;
  MIL << m-Match::NOCASE << endl;

  MIL << Match(8765) << endl;
  MIL << Match() << endl;


  INT << "===[END]============================================" << endl << endl;
  return 0;



  PoolQuery q;
  q.addString("foo*|k?");
  q.setMatchRegex();

  for_( it, q.nbegin(), q.nend() )
  {
    zypp::PoolItem pi( zypp::ResPool::instance().find( *it ) );
    MIL << pi.resolvable() << endl;
  }

  INT << "===[END]============================================" << endl << endl;
  return 0;
}

