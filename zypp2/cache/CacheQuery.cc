/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include <sqlite3.h>
#include "zypp2/cache/sqlite3x/sqlite3x.hpp"

#include "zypp/base/Logger.h"

#include "zypp2/cache/CacheInitializer.h"
#include "zypp2/cache/CacheQuery.h"

using namespace std;
using namespace zypp;
using namespace zypp::capability;
using namespace zypp::cache;
using namespace sqlite3x;

class CapabilityQuery::Impl
{
  public:
  Impl( sqlite3_connection &con )
    : _con(con), _read(false)
  {}
  
  sqlite3_connection &_con;
  sqlite3_reader _reader;
  bool _read;
};

CapabilityQuery::CapabilityQuery( Impl *impl )
  : _pimpl( impl)
{
    sqlite3_command cmd(_pimpl->_con, "select * from capabilities where resolvable_id=:id;");
    //cmd.bind(":id", id);
  
    sqlite3_reader reader = cmd.executereader();
    read();
}

bool CapabilityQuery::read()
{
  return ( _pimpl->_read = _pimpl->_reader.read() );
}
  
bool CapabilityQuery::valid() const
{
  return _pimpl->_read;
}

struct CacheQuery::Impl
{
  Impl()
  {}
  
  sqlite3_connection con;
};

CacheQuery::CacheQuery( const Pathname &dbdir )
    : _pimpl( new Impl() )
{
  cache::CacheInitializer initializer(dbdir, "zypp.db");
  if ( initializer.justInitialized() )
  {
    MIL << "database " << (dbdir + "zypp.db") << " was just created" << endl;
  }
  
  try
  {
    _pimpl->con.open( (dbdir + "zypp.db").asString().c_str());
    //_insert_resolvable_cmd = new sqlite3_command( *_con, INSERT_RESOLVABLE_QUERY );                                    
    //_insert_package_cmd = new sqlite3_command( *_con, INSERT_PACKAGE_QUERY );
  }
  catch(exception &ex)
  {
    //ZYPP_CAUGHT(ex);
    ZYPP_THROW(Exception(ex.what()));
  }
}

CapabilityQuery CacheQuery::createCapabilityQuery()
{
  return CapabilityQuery(new CapabilityQuery::Impl(_pimpl->con));
}

CacheQuery::~CacheQuery()
{
}