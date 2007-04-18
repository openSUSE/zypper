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
#include "zypp/base/Measure.h"
#include <sqlite3.h>
#include "zypp2/cache/sqlite3x/sqlite3x.hpp"
#include "zypp2/source/cached/CachedSourceImpl.h"
#include "zypp2/cache/QueryFactory.h"
#include "zypp2/cache/CapabilityQuery.h"
#include "zypp2/cache/sqlite_detail/CacheSqlite.h"
#include "zypp2/cache/DatabaseTypes.h"
#include "zypp2/cache/CacheCommon.h"
#include "zypp2/cache/sqlite_detail/QueryFactoryImpl.h"

#include "zypp2/source/cached/CachedSourcePackageImpl.h"
#include "zypp/detail/ResImplTraits.h"
#include "zypp/CapFactory.h"

using namespace zypp::detail;
using namespace zypp::cache;
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

void read_capabilities( sqlite3_connection &con, map<data::RecordId, NVRAD> &nvras );

void CachedSourceImpl::createResolvables(Source_Ref source_r)
{
  debug::Measure m("create resolvables");
  Pathname thePath = Pathname(url().getPathName()) + path();
  MIL << "Going to read dir " << thePath << std::endl;
  CapFactory capfactory;
  try {
    sqlite3_connection con((_dbdir + "zypp.db").asString().c_str());
    con.executenonquery("PRAGMA cache_size=8000;");
    con.executenonquery("BEGIN;");
    sqlite3_command cmd( con, "select id,name,version,release,epoch,arch,kind from resolvables;");
    map<data::RecordId, NVRAD> nvras;
    
    sqlite3_reader reader = cmd.executereader();
    while(reader.read())
    {
      long long id = reader.getint64(0);
      Dependencies deps;
      
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
      nvras[id] = dataCollect;
    }
    
    MIL << "Done reading resolvables nvra" << endl;
    
    read_capabilities( con, nvras);
    
    for ( map<data::RecordId, NVRAD>::const_iterator it = nvras.begin(); it != nvras.end(); ++it )
    {
      ResImplTraits<CachedSourcePackageImpl>::Ptr impl = new CachedSourcePackageImpl(selfSourceRef());
      Package::Ptr package = detail::makeResolvableFromImpl( it->second, impl );
      _store.insert (package);
    }
    con.executenonquery("COMMIT;");
  }
  catch(exception &ex) {
      cerr << "Exception Occured: " << ex.what() << endl;
   }
  //extract_packages_from_directory( _store, thePath, selfSourceRef(), true );
   
}

void read_capabilities( sqlite3_connection &con, map<data::RecordId, NVRAD> &nvras )
{
  CapFactory capfactory;
  // precompile statements
  
//   map<data::RecordId, capability::CapabilityImpl::Ptr> named_caps;
//   sqlite3_command select_named_cmd( con, "select v.id, c.refers_kind, n.name, v.version, v.release, v.epoch, v.relation named_capabilities v, capabilities c, names n where v.name_id=n.id and c.id=ncc.capability_id and ncc.named_capability_id=v.id;");
//   {
//     debug::Measure mnc("read named capabilities");
//     sqlite3_reader reader = select_named_cmd.executereader();
//     while  ( reader.read() )
//     {
//       
//     }
//   }
  sqlite3_command select_named_cmd( con, "select c.refers_kind, n.name, v.version, v.release, v.epoch, v.relation, c.dependency_type, c.resolvable_id from  names n , named_capabilities v, named_capabilities_capabilities ncc, capabilities c where v.name_id=n.id and c.id=ncc.capability_id and ncc.named_capability_id=v.id;");
  sqlite3_command select_file_cmd( con, "select c.refers_kind, dn.name, fn.name, c.dependency_type, c.resolvable_id from file_capabilities fc, capabilities c, files f, dir_names dn, file_names fn where f.id=fc.file_id and f.dir_name_id=dn.id and f.file_name_id=fn.id and c.id=fc.dependency_id;");
  
  {
    debug::Measure mnc("read named capabilities");
    sqlite3_reader reader = select_named_cmd.executereader();
    while  ( reader.read() )
    {
      Resolvable::Kind refer = db_kind2zypp_kind( static_cast<db::Kind>(reader.getint(0)) );
      zypp::Rel rel = db_rel2zypp_rel( static_cast<db::Rel>(reader.getint(5)) );
      data::RecordId rid = reader.getint64(7);
  
      if ( rel == zypp::Rel::NONE )
      {
        capability::NamedCap *ncap = new capability::NamedCap( refer, reader.getstring(1) );
        zypp::Dep deptype = db_deptype2zypp_deptype( static_cast<db::DependencyType>(reader.getint(5)) );
        
        nvras[rid][deptype].insert( capfactory.fromImpl( capability::CapabilityImpl::Ptr(ncap) ) ); 
      }
      else
      {
        capability::VersionedCap *vcap = new capability::VersionedCap( refer, reader.getstring(1), /* rel */ rel, Edition( reader.getstring(2), reader.getstring(3), reader.getint(4) ) );
        zypp::Dep deptype = db_deptype2zypp_deptype( static_cast<db::DependencyType>(reader.getint(5)) );
        nvras[rid][deptype].insert( capfactory.fromImpl( capability::CapabilityImpl::Ptr(vcap) ) ); 
      }
    }
  }
  
  {
    debug::Measure mnf("read file capabilities");
    sqlite3_reader reader = select_file_cmd.executereader();
    while  ( reader.read() )
    {
      Resolvable::Kind refer = db_kind2zypp_kind( static_cast<db::Kind>(reader.getint(0)) );
      capability::FileCap *fcap = new capability::FileCap( refer, reader.getstring(1) + "/" + reader.getstring(2) );
      zypp::Dep deptype = db_deptype2zypp_deptype( static_cast<db::DependencyType>(reader.getint(3)) );
      data::RecordId rid = reader.getint64(4);
      nvras[rid][deptype].insert( capfactory.fromImpl( capability::CapabilityImpl::Ptr(fcap) ) ); 
    }
  }
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
