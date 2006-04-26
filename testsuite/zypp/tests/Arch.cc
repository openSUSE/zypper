// Arch.cc
//
// tests for Arch
//

#include <iostream>
#include <list>
#include <string>

#include "zypp/base/Logger.h"
#include "zypp/Arch.h"

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
  Arch _arch32( "i386" );

  if (_arch32 != Arch_i386) return 1;

  if (_arch32.asString() != string("i386")) return 2;

  if (!_arch32.compatibleWith (Arch_x86_64)) return 3;

  if (arch_exception() != 0) return 4;

  if ( Arch() != Arch_noarch ) return 5;

  if ( Arch("") == Arch_noarch ) return 6;

  if ( ! Arch("").empty() ) return 7;

  if ( Arch_noarch.empty() ) return 8;

  if (_arch32.compare(Arch_x86_64) >= 0) return 9;

  return 0;
}
