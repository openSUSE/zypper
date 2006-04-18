#include <iostream>

#include "Tools.h"

#include "zypp/Bit.h"

using namespace std;
using namespace zypp;


void foreach( boost::function<void (int i, const char * t, double d)> f )
{
  f( 1, "2", 3.0 );
}

struct MyFn
{
  void operator()( int i, const char * t, double d ) const
  { MIL << __PRETTY_FUNCTION__ << endl; }
};


struct MyFn2
{
  MyFn2( string t )
  : _t( t )
  {}
  string _t;

  char operator()( long i, const char * t, double d ) const
  { MIL << t << __PRETTY_FUNCTION__ << endl; return 0; }
};


/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;

  foreach( MyFn() );
  foreach( MyFn2("fooo") );

  INT << "===[END]============================================" << endl << endl;
  return 0;
}

