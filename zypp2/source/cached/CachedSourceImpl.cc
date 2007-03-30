/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include <iostream>
#include "zypp/base/Logger.h"
#include "zypp2/cache/sqlite3x/sqlite3x.hpp"
#include "zypp2/source/cached/CachedSourceImpl.h"
#include "zypp2/cache/QueryFactory.h"
#include "zypp2/cache/CapabilityQuery.h"
#include "zypp2/cache/sqlite_detail/CacheSqlite.h"
#include "zypp2/cache/sqlite_detail/QueryFactoryImpl.h"

#include "zypp2/source/cached/CachedSourcePackageImpl.h"
#include "zypp/detail/ResImplTraits.h"
#include "zypp/CapFactory.h"

using namespace zypp::detail;
using std::endl;
using namespace std;
using namespace sqlite3x;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace source
{ /////////////////////////////////////////////////////////////////
 ///////////////////////////////////////////////////////////////////
namespace cached
{ /////////////////////////////////////////////////////////////////

CachedSourceImpl::CachedSourceImpl( const Pathname &dbdir)
  : _dbdir(dbdir)
{

}

CachedSourceImpl::~CachedSourceImpl()
{

}


void CachedSourceImpl::factoryInit()
{
  if ( ! ( (url().getScheme() == "file") || (url().getScheme() == "dir") ) )
  {
    ZYPP_THROW( Exception( "Plaindir only supports local paths, scheme [" + url().getScheme() + "] is not local" ) );
  }

  MIL << "Plaindir source initialized." << std::endl;
  MIL << "   Url      : " << url() << std::endl;
  MIL << "   Path     : " << path() << std::endl;
}

void CachedSourceImpl::createResolvables(Source_Ref source_r)
{
  Pathname thePath = Pathname(url().getPathName()) + path();
  MIL << "Going to read dir " << thePath << std::endl;
  CapFactory capfactory;
  try {
    sqlite3_connection_ptr con;
    con.reset( new sqlite3_connection((_dbdir + "zypp.db").asString().c_str()));
    sqlite3_command cmd( *con, "select id,name,version,release,epoch,arch,kind from resolvables;");
    
    cache::QueryFactory cachequery( new cache::QueryFactory::Impl( _dbdir, con ) );
    
    sqlite3_reader reader = cmd.executereader();
    while(reader.read())
    {
      long long id = reader.getint64(0);
      cache::CapabilityQuery capquery = cachequery.createCapabilityQuery(id);
      Dependencies deps;
          
      while ( capquery.read() )
      {
        Dependencies deps;
        std::pair<zypp::Dep, capability::CapabilityImpl::Ptr> d(capquery.value());
        zypp::Dep dep(d.first);
        Capability thecap = capfactory.fromImpl( capability::CapabilityImpl::Ptr(d.second) );
        deps[dep].insert(thecap);
      }

      Arch arch;
      string archstring = reader.getstring(5);
      if (!archstring.empty())
      arch = Arch(archstring);

      // Collect basic Resolvable data
      NVRAD dataCollect( reader.getstring(1),
                       Edition( reader.getstring(2), reader.getstring(3), reader.getint(4) ),
                       arch,
                       deps
                     );
      ResImplTraits<CachedSourcePackageImpl>::Ptr impl = new CachedSourcePackageImpl(selfSourceRef());
      Package::Ptr package = detail::makeResolvableFromImpl( dataCollect, impl );
      _store.insert (package);
    }
  }
  catch(exception &ex) {
      cerr << "Exception Occured: " << ex.what() << endl;
   }
  //extract_packages_from_directory( _store, thePath, selfSourceRef(), true );
}

      /////////////////////////////////////////////////////////////////
    } // namespace plaindir
    ///////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
