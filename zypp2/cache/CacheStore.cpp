
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
    //_insert_resolvable_cmd = new sqlite3_command( *_con, INSERT_RESOLVABLE_QUERY );                                    
    //_insert_package_cmd = new sqlite3_command( *_con, INSERT_PACKAGE_QUERY );
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

void CacheStore::consumePackage( const data::Package &package)
{}

data::RecordId CacheStore::appendResolvable( const Resolvable::Kind &kind, const NVRA &nvra, const data::Dependencies &deps )
{
  sqlite3_command *cmd = new sqlite3_command( *_con, "insert into resolvables ( name, version, release, epoch, arch, kind ) values ( :name, :version, :release, :epoch, :arch, :kind );" );
  cmd->bind( ":name", nvra.name );
  cmd->bind( ":version", nvra.edition.version() );
  cmd->bind( ":release", nvra.edition.release() );
  cmd->bind( ":epoch", static_cast<int>( nvra.edition.epoch() ) );
  cmd->bind( ":arch", zypp_arch2db_arch(nvra.arch) );
  cmd->bind( ":kind", db_kind2zypp_kind(kind) );
  
  _insert_resolvable_cmd->executenonquery();

  long long id = _con->insertid();
  
  appendDependencies( id, deps );
  
  return static_cast<data::RecordId>(id);
  return 1;
}

void CacheStore::appendDependencies( data::RecordId resolvable_id, const data::Dependencies &deps )
{
  appendDependency( resolvable_id, Dep::PROVIDES, deps.provides );
  appendDependency( resolvable_id, Dep::CONFLICTS, deps.conflicts );
//   appendDependency( resolvable_id, deps.obsoletes );
//   appendDependency( resolvable_id, deps.freshens );
//   appendDependency( resolvable_id, deps.requires );
//   appendDependency( resolvable_id, deps.prerequires );
//   appendDependency( resolvable_id, deps.recommends );
//   appendDependency( resolvable_id, deps.suggests );
//   appendDependency( resolvable_id, deps.supplements );
//   appendDependency( resolvable_id, deps.senhances );
}

void CacheStore::appendDependency( data::RecordId resolvable_id, zypp::Dep deptype, const data::DependencyList &deplist )
{

}


// data::RecordId CacheStore::insertResObject( const Resolvable::Kind &kind, const data::ResObject &res )
// {
//   _insert_resolvable_cmd->bind(1,  res.name.c_str(), -1);
//   _insert_resolvable_cmd->bind(2,  res.edition.version().c_str(), -1);
//   _insert_resolvable_cmd->bind(3,  res.edition.release().c_str(), -1);
//   _insert_resolvable_cmd->bind(4,  static_cast<int>( res.edition.epoch() ));
//   _insert_resolvable_cmd->bind(5,  zypp_arch2db_arch(res.arch));
//   _insert_resolvable_cmd->bind(6,  db_kind2zypp_kind(kind));
//   _insert_resolvable_cmd->bind(7,  res.summary.text().c_str(), -1);
//   _insert_resolvable_cmd->bind(8,  res.description.text().c_str(), -1);
//   _insert_resolvable_cmd->bind(9,  res.insnotify.c_str(), -1);
//   _insert_resolvable_cmd->bind(10, res.delnotify.c_str(), -1);
//   _insert_resolvable_cmd->bind(11, res.license_to_confirm.c_str(), -1);
//   _insert_resolvable_cmd->bind(12, res.vendor.c_str(), -1);
//   _insert_resolvable_cmd->bind(13, res.size ); // FIX cast?
//   _insert_resolvable_cmd->bind(14, res.archive_size ); // FIX cast?
//   _insert_resolvable_cmd->bind(15, res.source.c_str(), -1);
//   
//   _insert_resolvable_cmd->bind(16, res.source_media_nr);
//   _insert_resolvable_cmd->bind(17, static_cast<int>(res.install_only));
//   _insert_resolvable_cmd->bind(18, static_cast<int>(res.build_time) ); // FIX cast?
//   _insert_resolvable_cmd->bind(19, static_cast<int>(res.install_time) ); // FIX cast?
//   _insert_resolvable_cmd->executenonquery();
// 
//   return static_cast<data::RecordId>( _con->insertid() );
// }


}
}

