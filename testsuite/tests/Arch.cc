// Arch.cc
//
// tests for Arch
//

#include <iostream>
#include <list>
#include <string>
#include <zypp/base/Logger.h>

#include <zypp/Arch.h>

using namespace std;
using namespace zypp;

static int
arch_exception ()
{
  try {
    Arch _arch1(NULL);	// bad value, should raise exception
  } catch (exception exp) {
    return 0;		// exception raised
  }
  return 1;		// no exception
}

/******************************************************************
**
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
**
**      DESCRIPTION :
*/
int main( int argc, char * argv[] )
{
  Arch        _arch( "i386" );

  if (_arch != Arch_i386) return 1;

  if (arch_exception() != 0) return 1;

  return 0;
}
