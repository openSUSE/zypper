#include <iostream>
#include <fstream>
#include <zypp/base/Logger.h>
#include <zypp/detail/MessageImpl.h>
#include <zypp/Message.h>
#include <YUMParser.h>
#include <zypp/base/Logger.h>
#include <map>
#include "zypp/source/yum/YUMSource.h"


using namespace std;
using namespace zypp;
using namespace zypp::source::yum;


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
  YUMSource src;

  src.parseSourceMetadata("repodata");

  INT << "===[END]============================================" << endl;
  return 0;
}
