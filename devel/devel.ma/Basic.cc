#include "Tools.h"

#include <iostream>

#include <zypp/base/LogControl.h>
#include <zypp/base/LogTools.h>
#include <zypp/base/String.h>

using namespace std;
using namespace zypp;

struct BuildRes
{
  BuildRes()
  {
  }

  void operator()( const char *const & line_r ) const
  { operator()( string( line_r ) ); }

  void operator()( const string & line_r ) const
  {
    vector<string> words;
    switch ( str::split( line_r, back_inserter(words) ) )
      {
      default:
        DBG << words << endl;
      }
  }

};


/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;

  //ResPool pool( getZYpp()->pool() );

  const char *const lines[] = {
    "I Product prod 1 1 x86_64"
    "A Product prod2 1 1 x86_64"
  };

  BuildRes a;

  for_each( lines, lines+(sizeof(lines)/sizeof(const char *const)),
            a );

  INT << "===[END]============================================" << endl
      << endl;
  return 0;
}
