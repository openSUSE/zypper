
#include "zypp2/cache/ResolvableQuery.h"
#include "zypp2/cache/DatabaseTypes.h"
#include "zypp2/cache/sqlite3x/sqlite3x.hpp"

using namespace sqlite3x;
using namespace std;

namespace zypp { namespace cache {
  

struct ResolvableQuery::Impl
{
  Impl( const Pathname &dbdir)
  : _dbdir(dbdir)
  {
    _fields = "id, name, version, release, epoch, arch, kind, insnotify, delnotify, license_to_confirm, vendor, installed_size, archive_size, install_only, build_time, install_time, catalog_id";
  }
  
  ~Impl()
  {
  }
  
  data::ResObject_Ptr fromRow( sqlite3_reader &reader )
  {
    data::ResObject_Ptr ptr (new data::ResObject);
    
    ptr->name = reader.getstring(1);
    ptr->edition = Edition( reader.getstring(2), reader.getstring(3), reader.getint(4));
    ptr->arch = db_arch2zypp_arch( static_cast<db::Arch>(reader.getint(5)));
    
    return ptr;
  }
  
  void query( const data::RecordId &id,
                  ProcessResolvable fnc )
  {
    sqlite3_connection con((_dbdir + "zypp.db").asString().c_str());
    //con.executenonquery("PRAGMA cache_size=8000;");
    con.executenonquery("BEGIN;");
    sqlite3_command cmd( con, "select " + _fields + " from resolvables where id=:id;");
    cmd.bind(":id", id);
    sqlite3_reader reader = cmd.executereader();
    while(reader.read())
    {
      fnc( id, fromRow(reader) );
    }
    con.executenonquery("COMMIT;");
  }
        
  void query( const std::string &s,
              ProcessResolvable fnc  )
  {
    sqlite3_connection con((_dbdir + "zypp.db").asString().c_str());
    //con.executenonquery("PRAGMA cache_size=8000;");
    con.executenonquery("BEGIN;");
    sqlite3_command cmd( con, "select " + _fields + " from resolvables where name like '%:name%';");
    cmd.bind(":name", s);
    sqlite3_reader reader = cmd.executereader();
    while(reader.read())
    {
      fnc( reader.getint64(0), fromRow(reader) );
    }
    con.executenonquery("COMMIT;");
  }
  
  Pathname _dbdir;
  string _fields;
};
  
ResolvableQuery::ResolvableQuery( const Pathname &dbdir)
  : _pimpl(new Impl(dbdir))
{
}
      
void ResolvableQuery::query( const data::RecordId &id, ProcessResolvable fnc  )
{
  _pimpl->query(id, fnc);
}
      
void ResolvableQuery::query( const std::string &s, ProcessResolvable fnc  )
{
  _pimpl->query(s, fnc);
}
  

} } // namespace zypp::cache
