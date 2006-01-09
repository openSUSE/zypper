#include <iostream>
#include <ctime>

#include <zypp/base/Logger.h>
#include <zypp/base/Exception.h>

#include <zypp/CapMatch.h>

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

  CapMatch u( CapMatch::irrelevant );
  CapMatch y(true);
  CapMatch n(false);

#define OUT(X) DBG << #X << " " << (X) << endl

  OUT( u );
  OUT( u == y );
  OUT( u != y );

  OUT( u == true );
  OUT( u == false );
  OUT( true == u );
  OUT( false == u );

  OUT( u && y );
  OUT( u && n );
  OUT( u && u );

  OUT( y && y );
  OUT( y && n );
  OUT( y && u );

  OUT( n && y );
  OUT( n && n );
  OUT( n && u );

  OUT( u || y );
  OUT( u || n );
  OUT( u || u );

  OUT( y || y );
  OUT( y || n );
  OUT( y || u );

  OUT( n || y );
  OUT( n || n );
  OUT( n || u );

  OUT( !u );
  OUT( !y );
  OUT( !n );

  INT << "===[END]============================================" << endl;
  return 0;
}
