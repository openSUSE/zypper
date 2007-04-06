#include <iostream>
#include <fstream>
#include <map>
#include "zypp/base/Logger.h"
#include "zypp/SourceFactory.h"
#include "zypp/Source.h"
#include "zypp/source/SourceImpl.h"
#include "zypp/SourceCache.h"
#include "zypp/Package.h"
#include "zypp/Message.h"

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
//  Url url = Url("ftp://cml.suse.cz/netboot/find/SUSE-10.1-CD-OSS-i386-Beta1-CD1");
//  Url url = Url("http://users.suse.cz/~jsrain/devel.jsrain");
//  Url url = Url("dir:/local/zypp/libzypp/devel/devel.jsrain");
  Url url = Url("http://armstrong.suse.de/download/Code/10/update/i386/");
  Source_Ref s = _f.createFrom( url, p );
  ResStore store = s.resolvables();
  for (ResStore::const_iterator it = store.begin();
       it != store.end(); it++)
  {
    ERR << **it << endl;
    if (isKind<Message>(*it))
    {
      ERR << "Message" << endl;
      Message::constPtr pkg = boost::dynamic_pointer_cast<const Message>( *it );
      ERR << "Patch PTR: " << pkg->patch() << endl;
      if (pkg->patch())
        ERR << "Patch: " << *(pkg->patch()) << endl;
    }
  }
//  SourceCache().storeSource(s);
//  ERR << store << endl;
  INT << "===[END]============================================" << endl;
  return 0;
}
