#include <iostream>
#include <ctime>

#include <fstream>
#include <list>
#include <string>
#include <zypp/base/Logger.h>
#include <zypp/base/String.h>
#include <zypp/base/IOStream.h>
#include <zypp/base/PtrTypes.h>

#include <zypp/ByteCount.h>
#include <zypp/CapFactory.h>
#include <zypp/CapSet.h>

using namespace std;
using namespace zypp;


void test( long long v )
{
  ByteCount b( v );
  unsigned fw = 7;
  unsigned sw = 1;
  unsigned p  = 2;
  MIL << "'" << b << "'\t"
      << "'" << b.asString( ByteCount::B, fw, sw, p ) << "'\t"
      << "'" << b.asString( ByteCount::K, fw, sw, p ) << "'\t"
      << "'" << b.asString( ByteCount::M, fw, sw, p ) << "'\t"
      << "'" << b.asString( ByteCount::G, fw, sw, p ) << "'\t"
      << "'" << b.asString( ByteCount::T, fw, sw, p ) << "'" << endl;
}
void test2( long long v )
{
  ByteCount b( v );
  unsigned fw = 7;
  unsigned sw = 1;
  unsigned p  = 2;
  MIL << b.asString( ByteCount::B ) << "-----------------" << endl;
  MIL << "'" << b.fullBlocks( ByteCount::B ) << "'\t"
      << "'" << b.blocks( ByteCount::B ) << endl;
  MIL << "'" << b.fullBlocks( ByteCount::K ) << "'\t"
      << "'" << b.blocks( ByteCount::K ) << endl;
}

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;

  for ( long long i = 1; i < 10000000000000LL; i*=2 )
    {
      test( i );
    }

  test2( -1025 );
  test2( -1024 );
  test2( -2 );
  test2( -1 );
  test2( 0 );
  test2( 1 );
  test2( 2 );
  test2( 1024 );
  test2( 1025 );

  INT << "===[END]============================================" << endl;
  return 0;
}

