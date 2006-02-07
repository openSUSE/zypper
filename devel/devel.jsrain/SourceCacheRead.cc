#include <iostream>
#include <fstream>
#include <map>
#include "zypp/base/Logger.h"
#include "zypp/SourceFactory.h"
#include "zypp/Source.h"
#include "zypp/source/SourceImpl.h"
#include "zypp/SourceCache.h"
#include "zypp/SourceManager.h"

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
  SourceCache().restoreSources();
  Source_Ref s = SourceManager::sourceManager()->findSource(0);
  ResStore store = s.resolvables();
  for (ResStore::const_iterator it = store.begin();
       it != store.end(); it++)
  {
    ERR << **it << endl;
  }
 
  return 0;
}
