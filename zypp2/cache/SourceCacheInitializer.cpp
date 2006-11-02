/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include <vector>

#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp2/cache/SourceCacheInitializer.h"
#include "zypp/target/store/PersistentStorage.h"
#include "zypp2/cache/Utils.h"

#define ZYPP_DB_FILE "/var/lib/zypp/zypp.db"

using namespace sqlite3x;
using namespace std;

//////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace cache
{ /////////////////////////////////////////////////////////////////
  

static const char * SOURCES_TABLE_SCHEMA = "create table sources ( id integer primary key autoincrement, alias varchar unique, type varchar, description varchar,  url varchar, path varchar,  enabled integer, autorefresh integer, timestamp integer, checksum varchar);";

static const char * RESOLVABLES_TABLE_SCHEMA = "\
 CREATE TABLE resolvables (   id INTEGER PRIMARY KEY AUTOINCREMENT,   name VARCHAR,   version VARCHAR,   release VARCHAR,   epoch INTEGER,   arch INTEGER,  installed_size INTEGER,  catalog VARCHAR,  installed INTEGER,  local INTEGER,  status INTEGER,  category VARCHAR,  license VARCHAR,  kind INTEGER); \
 CREATE INDEX resolvable_sources ON resolvables (sources); \
 CREATE INDEX resolvable_name ON resolvables (name); \
 CREATE INDEX resolvable_spec ON resolvables (name, version, release, epoch, arch); \
 CREATE TRIGGER remove_resolvables AFTER DELETE ON resolvables  BEGIN    DELETE FROM package_details WHERE resolvable_id = old.id; \
 DELETE FROM patch_details WHERE resolvable_id = old.id; \
 DELETE FROM pattern_details WHERE resolvable_id = old.id; \
 DELETE FROM product_details WHERE resolvable_id = old.id; \
 DELETE FROM message_details WHERE resolvable_id = old.id;   \
 DELETE FROM script_details WHERE resolvable_id = old.id; \
 DELETE FROM dependencies WHERE resolvable_id = old.id; \
 DELETE FROM files WHERE resolvable_id = old.id;  END;";

static const char * PACKAGE_DETAILS_TABLE_SCHEMA = "\
    CREATE TABLE package_details (   resolvable_id INTEGER NOT NULL,   rpm_group VARCHAR,  summary VARCHAR,  description VARCHAR,  package_url VARCHAR,  package_filename VARCHAR,  signature_filename VARCHAR,  file_size INTEGER,  install_only INTEGER,  media_nr INTEGER); \
    CREATE INDEX package_details_resolvable_id ON package_details (resolvable_id);\
    CREATE VIEW packages AS SELECT * FROM resolvables, package_details WHERE resolvables.id = package_details.resolvable_id;";

static const char * MESSAGE_DETAILS_TABLE_SCHEMA = "\
CREATE TABLE message_details (resolvable_id INTEGER NOT NULL, content VARCHAR);";

static const char * PATTERN_DETAILS_TABLE_SCHEMA = "\
CREATE TABLE pattern_details (resolvable_id INTEGER NOT NULL, summary VARCHAR, description VARCHAR);";

static const char * PRODUCT_DETAILS_TABLE_SCHEMA = "\
CREATE TABLE product_details (resolvable_id INTEGER NOT NULL, summary VARCHAR, description VARCHAR );";

static const char * SCRIPT_DETAILS_TABLE_SCHEMA = "\
CREATE TABLE script_details (resolvable_id INTEGER NOT NULL, do_script VARCHAR, undo_script VARCHAR);";
    
static const char * PATCH_DETAILS_TABLE_SCHEMA = "\
CREATE TABLE patch_details (resolvable_id INTEGER NOT NULL, patch_id VARCHAR, creation_time INTEGER, reboot INTEGER, restart INTEGER, interactive INTEGER,     summary VARCHAR, description VARCHAR);";

static const char * DEPENDENCIES_TABLE_SCHEMA = "\
    CREATE TABLE dependencies ( resolvable_id INTEGER NOT NULL, dep_type INTEGER NOT NULL, name VARCHAR, version VARCHAR, release VARCHAR, epoch INTEGER, arch INTEGER, relation INTEGER, dep_target INTEGER);";

 

// id 0, alias 1 , type 2, desc 3, url 4, path 5, enabled 6, autorefresh 7, timestamp 8, checksum 9
  
SourceCacheInitializer::SourceCacheInitializer( const Pathname &root_r, const Pathname &db_file )
  : _root(root_r), _just_initialized(false)
{
  try
  {
    _con.reset( new sqlite3_connection( (_root + db_file).asString().c_str()) );
  }
  catch(exception &ex) {
    ERR << "Exception Occured: " << ex.what() << endl;
  } 

  try
  {
    if( ! tablesCreated() )
    {
      createTables();
      _just_initialized = true;
      _con->close();
      MIL << "Source cache initialized" << std::endl;
    }
    else
    {
      MIL << "Source cache already initialized" << std::endl;
    }
  }
  catch(exception &ex)
  {
    ERR << "Exception Occured: " << ex.what() << endl;
  }
  
}

bool SourceCacheInitializer::justInitialized() const
{
  return _just_initialized;
}

SourceCacheInitializer::~SourceCacheInitializer()
{
 
}

bool SourceCacheInitializer::tablesCreated() const
{
  unsigned int count = _con->executeint("select count(*) from sqlite_master where type='table';");
  return ( count == 1 );
}

void SourceCacheInitializer::createTables()
{
  sqlite3_transaction trans(*_con);
  {
    _con->executenonquery(SOURCES_TABLE_SCHEMA);
    _con->executenonquery(RESOLVABLES_TABLE_SCHEMA);
    _con->executenonquery(PACKAGE_DETAILS_TABLE_SCHEMA);
    _con->executenonquery(PRODUCT_DETAILS_TABLE_SCHEMA);
    _con->executenonquery(PATCH_DETAILS_TABLE_SCHEMA);
    _con->executenonquery(SCRIPT_DETAILS_TABLE_SCHEMA);
    _con->executenonquery(MESSAGE_DETAILS_TABLE_SCHEMA);
    _con->executenonquery(PATTERN_DETAILS_TABLE_SCHEMA);
    _con->executenonquery(DEPENDENCIES_TABLE_SCHEMA);
  }
  
  trans.commit();
}

std::ostream & SourceCacheInitializer::dumpOn( std::ostream & str ) const
{
  return str;
}

}
}

