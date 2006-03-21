#include <iostream>
#include <list>
#include <set>

#include "Tools.h"

#include <zypp/base/LogControl.h>
#include <zypp/base/GzStream.h>
#include <zypp/base/IOStream.h>

using namespace std;
using namespace zypp;

std::ostream & operator<<( std::ostream & str, std::istream & obj )
{
  return str << "IN("
             << ( obj.good() ? 'g' : '_' )
             << ( obj.eof()  ? 'e' : '_' )
             << ( obj.fail() ? 'F' : '_' )
             << ( obj.bad()  ? 'B' : '_' )
             << ' ' << obj.tellg() << ")";
}
std::ostream & operator<<( std::ostream & str, std::ostream & obj )
{
  return str << "OUT("
             << ( obj.good() ? 'g' : '_' )
             << ( obj.eof()  ? 'e' : '_' )
             << ( obj.fail() ? 'F' : '_' )
             << ( obj.bad()  ? 'B' : '_' )
             << ' ' << obj.tellp() << ")";
}


/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;

  ifgzstream inf( "file" );
  ofgzstream zout( "file.gz" );
  MIL << inf << endl;
  MIL << zout << endl;

  inf >> noskipws;
  copy( istream_iterator<char>(inf), istream_iterator<char>(),
        ostream_iterator<char>(zout) );
  MIL << inf << endl;
  MIL << zout << endl;
  zout.close();
  MIL << zout << endl;

  ifgzstream zinf( "file.gz" );
  DBG << zinf << endl;
  zinf >> noskipws;
  copy( istream_iterator<char>(zinf), istream_iterator<char>(),
        ostream_iterator<char>(DBG) );
  DBG << endl;
  MIL << zinf << endl;

  INT << "===[END]============================================" << endl << endl;
  return 0;
}

