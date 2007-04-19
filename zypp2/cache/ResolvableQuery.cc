
#include "zypp2/cache/ResolvableQuery.h"
#include "zypp2/cache/sqlite3x/sqlite3x.hpp"

using namespace sqlite3x;

namespace zypp { namespace cache {
  

struct ResolvableQuery::Impl
{
  Impl( const Pathname &dbdir, ProcessResolvable fnc )
  : _dbdir(dbdir), _fnc(fnc)
  {}
  
  ~Impl()
  {
  }
  
  
  void query( const data::RecordId &id )
  {
    sqlite3_connection con((_dbdir + "zypp.db").asString().c_str());
    //con.executenonquery("PRAGMA cache_size=8000;");
    con.executenonquery("BEGIN;");
    sqlite3_command cmd( con, "select * from resolvables where id=:id;");
    cmd.bind(":id", id);
    sqlite3_reader reader = cmd.executereader();
    while(reader.read())
    {
      
    }
    con.executenonquery("COMMIT;");
  }
        
  void query( const std::string &s )
  {
    
  }
  
  Pathname _dbdir;
  ProcessResolvable _fnc;
};
  
ResolvableQuery::ResolvableQuery( const Pathname &dbdir, ProcessResolvable fnc )
  : _pimpl(new Impl(dbdir, fnc))
{
}
      
void ResolvableQuery::query( const data::RecordId &id )
{
  _pimpl->query(id);
}
      
void ResolvableQuery::query( const std::string &s )
{
  _pimpl->query(s);
}
  

} } // namespace zypp::cache
