#include <iostream>
#include <fstream>
#include <map>
#include "zypp/base/Logger.h"
#include "zypp/SourceFactory.h"
#include "zypp/Source.h"
#include "zypp/source/SourceImpl.h"

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
  SourceFactory _f;
  Pathname p = "/";
  Url url = Url("ftp://cml.suse.cz/netboot/find/SUSE-10.1-CD-OSS-i386-Alpha4-CD1");
//  Url url = Url("http://lide.suse.cz/~~jsrain/devel.jsrain");
//  Url url = Url("dir:/local/zypp/libzypp/devel/devel.jsrain");
  Source s = _f.createFrom( url, p );
  ResStore store = s.resolvables();
  for (ResStore::const_iterator it = store.begin();
       it != store.end(); it++)
  {
    ERR << **it << endl;
  }
  ERR << store << endl;
  INT << "===[END]============================================" << endl;
  return 0;
}
