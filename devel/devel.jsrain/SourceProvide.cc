#include <iostream>
#include <fstream>
#include <map>
#include "zypp/base/Logger.h"
#include "zypp/SourceManager.h"
#include "zypp/Source.h"
#include "zypp/source/SourceImpl.h"
#include "zypp/ResPoolManager.h"

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
  SourceManager_Ptr mgr = SourceManager::sourceManager();
  ResPoolManager pool;
  unsigned id = mgr->addSource(pool, Url("ftp://cml.suse.cz/netboot/find/SUSE-10.1-CD-OSS-i386-Beta1-CD1"), Pathname("/"));
  ERR << id << endl;
  ERR << pool << endl;
  Source & src = mgr->findSource(id);
  Pathname p = src.provideFile("/control.xml", 0);
  ERR << p << endl;
  mgr->removeSource(pool, id);
  ERR << pool << endl;
  INT << "===[END]============================================" << endl;
  return 0;
}
