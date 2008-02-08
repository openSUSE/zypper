#include <sys/time.h>

#include <iostream>
#include <fstream>

#include <zypp/base/Logger.h>
#include <zypp/ZYpp.h>
#include <zypp/ZYppFactory.h>

#include "zypp/Product.h"
#include "zypp/Package.h"

#include "zypp/TmpPath.h"
#include "zypp/ProgressData.h"
#include "zypp/parser/yum/RepoParser.h"
#include "zypp/repo/yum/Downloader.h"

#include "zypp/sat/Pool.h"

#include "zypp/PoolQuery.h"

using namespace std;
using namespace zypp;
using namespace zypp::repo;

bool result_cb( const ResObject::Ptr &r )
{
  cout << r << endl;
}

int main(int argc, char **argv)
{
    try
    {
      ZYpp::Ptr z = getZYpp();
    
      //z->initializeTarget("/");
      //z->target()->load();

      sat::Pool::instance().addRepoSolv("./foo.solv");

//       for ( ResPool::const_iterator it = z->pool().begin(); it != z->pool().end(); ++it )
//       {
//         ResObject::constPtr res = it->resolvable();
//         if ( res->name() == "kde4-kcolorchooser")
//         {
//           cout << res << endl;
//           cout << res->summary() << " | " << res->size() << endl;
//         }
//       }

      PoolQuery query();
      query.execute("kde", &result_cb);
      

    }
    catch ( const Exception &e )
    {
      ZYPP_CAUGHT(e);
      cout << e.msg() << endl;
    }
    
    return 0;
}



