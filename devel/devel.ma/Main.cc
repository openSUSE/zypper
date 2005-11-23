#include <iostream>
#include <iterator>
#include <functional>
#include <set>
#include <algorithm>
#include <zypp/base/Logger.h>
#include <zypp/Arch.h>
#include <zypp/Edition.h>
#include <zypp/Rel.h>

using namespace std;


using namespace zypp;
/******************************************************************
**
**
**	FUNCTION NAME : main
**	FUNCTION TYPE : int
**
**	DESCRIPTION :
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;

  DBG << Edition( "1.0", "1a" ) << endl;
  DBG << Edition( "2.0", "13" ) << endl;
  DBG << Edition( "2_0", "13" ) << endl;
  DBG << Edition( "3.0", "13.4" ) << endl;



  INT << "===[END]============================================" << endl;
  return 0;
}

