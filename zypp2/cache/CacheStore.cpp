
#include <sqlite3.h>

#include "zypp/ZYppFactory.h"
#include "zypp/ZYpp.h"

#include "zypp2/cache/CacheInitializer.h"
#include "zypp2/cache/CacheStore.h"
#include "zypp2/cache/DatabaseTypes.h"

using namespace std;
using namespace zypp;
using namespace zypp::cache;
using namespace sqlite3x;

#define INSERT_RESOLVABLE_QUERY \
  "insert into resolvables (" \
  "name, version, release" \
  ", epoch, arch, kind" \
  ", summary, description" \
  ", insnotify, delnotify" \
  ", license_to_confirm, vendor, size, archive_size" \
  ", catalog, catalog_media_nr, install_only" \
  ", build_time, install_time )" \
  " values ( ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ? );"

#define INSERT_PACKAGE_QUERY \
  "insert into package_details (" \
  " resolvable_id, checksum, changelog, buildhost, distribution, license" \
  " , packager, package_group, url, os, prein, postin, preun, postun, source_size" \
  " , authors, filenames, location ) values ( ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ? );"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace cache
{ /////////////////////////////////////////////////////////////////

CacheStore::CacheStore( const Pathname &dbdir )
{
  cache::CacheInitializer initializer(dbdir, "zypp.db");
  if ( initializer.justInitialized() )
  {
    MIL << "database " << (dbdir + "zypp.db") << " was just created" << endl;
  }
  
  try
  {
    _con.reset( new sqlite3_connection( (dbdir + "zypp.db").asString().c_str()) );
    _insert_resolvable_cmd = new sqlite3_command( *_con, INSERT_RESOLVABLE_QUERY );                                    
    _insert_package_cmd = new sqlite3_command( *_con, INSERT_PACKAGE_QUERY );
  }
  catch(exception &ex)
  {
    //ZYPP_CAUGHT(ex);
    ZYPP_THROW(Exception(ex.what()));
  }
  
  // disable autocommit
  _con->executenonquery("BEGIN;");
}

CacheStore::CacheStore()
{
  CacheStore( getZYpp()->homePath() );
}

CacheStore::~CacheStore()
{
  _con->executenonquery("COMMIT;");
}

void CacheStore::consumePackage( const data::Package &package )
{
  data::RecordId id = insertResObject( ResTraits<Package>::kind, package );
}

data::RecordId CacheStore::insertResObject( const Resolvable::Kind &kind, const data::ResObject &res )
{
  _insert_resolvable_cmd->bind(1,  res.name.c_str(), -1);
  _insert_resolvable_cmd->bind(2,  res.edition.version().c_str(), -1);
  _insert_resolvable_cmd->bind(3,  res.edition.release().c_str(), -1);
  _insert_resolvable_cmd->bind(4,  static_cast<int>( res.edition.epoch() ));
  _insert_resolvable_cmd->bind(5,  zypp_arch2db_arch(res.arch));
  _insert_resolvable_cmd->bind(6,  db_kind2zypp_kind(kind));
  _insert_resolvable_cmd->bind(7,  res.summary.text().c_str(), -1);
  _insert_resolvable_cmd->bind(8,  res.description.text().c_str(), -1);
  _insert_resolvable_cmd->bind(9,  res.insnotify.c_str(), -1);
  _insert_resolvable_cmd->bind(10, res.delnotify.c_str(), -1);
  _insert_resolvable_cmd->bind(11, res.license_to_confirm.c_str(), -1);
  _insert_resolvable_cmd->bind(12, res.vendor.c_str(), -1);
  _insert_resolvable_cmd->bind(13, res.size ); // FIX cast?
  _insert_resolvable_cmd->bind(14, res.archive_size ); // FIX cast?
  _insert_resolvable_cmd->bind(15, res.source.c_str(), -1);
  
  _insert_resolvable_cmd->bind(16, res.source_media_nr);
  _insert_resolvable_cmd->bind(17, static_cast<int>(res.install_only));
  _insert_resolvable_cmd->bind(18, static_cast<int>(res.build_time) ); // FIX cast?
  _insert_resolvable_cmd->bind(19, static_cast<int>(res.install_time) ); // FIX cast?
  _insert_resolvable_cmd->executenonquery();

  return static_cast<data::RecordId>( _con->insertid() );
}

void CacheStore::insertPackage( data::RecordId id, const data::Package &pkg )
{
  sqlite_int64 sqid = static_cast<sqlite_int64>(id);
  /*
  " resolvable_id, checksum, changelog, buildhost, distribution, license" \
  " , packager, package_group, url, os, prein, postin, preun, postun, source_size" \
  " , authors, filenames, location ) values ( ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ? );"
  */
  _insert_package_cmd->bind( 1, sqid );
  _insert_package_cmd->bind( 2 );
  //_insert_package_cmd->bind( 2, pkg.checksum.asString().c_str() );
  _insert_package_cmd->bind( 3 );
  _insert_package_cmd->bind( 4, pkg.buildhost.c_str() );
}



}
}

