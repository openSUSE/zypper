#include <sys/time.h>

#include <iostream>
#include <fstream>

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

#include <zypp/base/Logger.h>
#include <zypp/ZYpp.h>
#include <zypp/ZYppFactory.h>

#include "zypp/Product.h"
#include "zypp/Package.h"

#include "zypp/TmpPath.h"

#include "zypp/sat/Pool.h"

#include "zypp/PoolQuery.h"

using namespace std;
using namespace zypp;
using namespace zypp::repo;

bool result_cb( const ResObject::Ptr &r )
{
  cout << r << endl;
}

boost::mutex io_mutex;

struct Counter
{
  Counter(int id) : id(id) { }
  void operator()()
  {
    for (int i = 0; i < 10; ++i)
    {
      //boost::mutex::scoped_lock lock(io_mutex);
      std::cout << id << ": " << i << std::endl;
      if ( i == 4 )
      {
        boost::thread thrd2(Counter(3));
      }
    }
  }
  int id;
};

int main(int argc, char **argv)
{
    try
    {
      ZYpp::Ptr z = getZYpp();
    
      boost::thread thrd1(Counter(1));
      boost::thread thrd2(Counter(2));
      thrd1.join();
      thrd2.join();
      return 0;
      
      //z->initializeTarget("/");
      //z->target()->load();

//      sat::Pool::instance().addRepoSolv("./foo.solv");

//       for ( ResPool::const_iterator it = z->pool().begin(); it != z->pool().end(); ++it )
//       {
//         ResObject::constPtr res = it->resolvable();
//         if ( res->name() == "kde4-kcolorchooser")
//         {
//           cout << res << endl;
//           cout << res->summary() << " | " << res->size() << endl;
//         }
//       }

    }
    catch ( const Exception &e )
    {
      ZYPP_CAUGHT(e);
      cout << e.msg() << endl;
    }
    
    return 0;
}



