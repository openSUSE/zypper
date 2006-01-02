#include <iostream>
#include <ctime>

#include <zypp/base/Logger.h>
#include <zypp/base/Exception.h>

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

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  --argc;
  ++argv;
  if ( ! argc )
    {
      cerr << "Usage: prognme <packages file>" << endl;
      return 1;
    }
  string file( argv[0] );

  INT << "===[START]==========================================" << endl;


  INT << "===[END]============================================" << endl;
  return 0;
}
