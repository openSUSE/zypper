#include <iostream>
#include <ctime>

#include <zypp/base/Logger.h>
#include <zypp/base/Exception.h>

#include <zypp/Edition.h>

using namespace std;
using namespace zypp;

// work around flaw in y2logview
template<class _Tp>
  void printOnHack( const _Tp & obj )
  {
    MIL << obj << endl;
  };

///////////////////////////////////////////////////////////////////
// Just for the stats
struct Measure
{
  time_t _begin;
  Measure()
  : _begin( time(NULL) )
  {
    USR << "START MEASURE..." << endl;
  }
  ~Measure()
  {
    USR << "DURATION: " << (time(NULL)-_begin) << " sec." << endl;
  }
};

///////////////////////////////////////////////////////////////////
// Print stream status
ostream & operator<<( ostream & str, const istream & obj ) {
  return str
  << (obj.good() ? 'g' : '_')
  << (obj.eof()  ? 'e' : '_')
  << (obj.fail() ? 'F' : '_')
  << (obj.bad()  ? 'B' : '_');
}

namespace zypp
{



}

using namespace zypp;

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;

  Edition::Range any;
  DBG << any << endl;

  Edition l( "1.0" );
  Edition r( "2.0" );

#define R(O,E) Edition::Range( Rel::O, E )

#define NONE(E) R(NONE,E)
#define ANY(E) R(ANY,E)
#define LT(E) R(LT,E)
#define LE(E) R(LE,E)
#define EQ(E) R(EQ,E)
#define GE(E) R(GE,E)
#define GT(E) R(GT,E)
#define NE(E) R(NE,E)

#define OV(L,R) DBG << #L << " <> " << #R << " ==> " << Edition::Range::overlaps( L, R ) << endl

  ERR << "Omitting Rel::NE" << endl;

#define OVALL( L )  \
  DBG << "----------------------------" << endl; \
  OV( L, NONE(r) ); \
  OV( L, ANY(r) );  \
  OV( L, LT(r) );   \
  OV( L, LE(r) );   \
  OV( L, EQ(r) );   \
  OV( L, GE(r) );   \
  OV( L, GT(r) );   \
  DBG << "----------------------------" << endl;

  OVALL( NONE(l) );
  OVALL( ANY(l) );
  OVALL( LT(l) );
  OVALL( LE(l) );
  OVALL( EQ(l) );
  OVALL( GE(l) );
  OVALL( GT(l) );

  // same for l > r and l == r

  INT << "===[END]============================================" << endl;
  return 0;
}
