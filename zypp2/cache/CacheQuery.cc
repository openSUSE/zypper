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

#include "zypp2/cache/DatabaseTypes.h"
#include "zypp2/cache/CacheInitializer.h"
#include "zypp2/cache/CacheQuery.h"

using namespace std;
using namespace zypp;
using namespace zypp::capability;
using namespace zypp::cache;
using namespace sqlite3x;

typedef shared_ptr<sqlite3_command> sqlite3_command_ptr;
typedef shared_ptr<sqlite3_reader> sqlite3_reader_ptr;

struct DatabaseContext
{
  sqlite3_connection con;
  sqlite3_command_ptr select_vercap_cmd;
  Pathname dbdir;
};

typedef shared_ptr<DatabaseContext> DatabaseContext_Ptr;

class CapabilityQuery::Impl
{
  public:
  Impl( DatabaseContext_Ptr p_context, const zypp::Dep &deptype, const data::RecordId &resolvable_id  )
    : context(p_context), _read(false), _deptype(deptype), _resolvable_id(resolvable_id)
  {}
  
  ~Impl()
  {
    MIL << endl;
  }
  
  DatabaseContext_Ptr context;
  sqlite3_reader_ptr _reader;
  bool _read;
  zypp::Dep _deptype;
  data::RecordId _resolvable_id;
};

CapabilityQuery::CapabilityQuery( Impl *impl)
  : _pimpl( impl)
{
    //cmd.bind(":id", id);
    _pimpl->context->select_vercap_cmd->bind(":rid", _pimpl->_resolvable_id);
    _pimpl->context->select_vercap_cmd->bind(":dtype", static_cast<int>( zypp_deptype2db_deptype(_pimpl->_deptype) ) );
    _pimpl->_reader.reset( new sqlite3_reader(_pimpl->context->select_vercap_cmd->executereader()));
    MIL << "Done setup query" << endl;
    read();
    MIL << "Done first read" << endl;
}

CapabilityQuery::~CapabilityQuery()
{
  //MIL << endl;
  // it has to be disposed before the commands
  _pimpl->_reader.reset();
}

bool CapabilityQuery::read()
{
  //MIL << endl;
  return ( _pimpl->_read = _pimpl->_reader->read() );
}
  
bool CapabilityQuery::valid() const
{
  return _pimpl->_read;
}

CapabilityImpl::Ptr CapabilityQuery::value()
{
  Resolvable::Kind refer = db_kind2zypp_kind( static_cast<db::Kind>(_pimpl->_reader->getint(0)) );
  //Resolvable::Kind refer = ResTraits<Package>::kind;
  zypp::Rel rel = db_rel2zypp_rel( static_cast<db::Rel>(_pimpl->_reader->getint(5)) );
  //zypp::Rel rel = Rel::EQ;
  capability::VersionedCap *vcap = new capability::VersionedCap( refer, _pimpl->_reader->getstring(1), /* rel */ rel, Edition( _pimpl->_reader->getstring(2), _pimpl->_reader->getstring(3), _pimpl->_reader->getint(4) ) );
  return capability::VersionedCap::Ptr(vcap);
}

struct CacheQuery::Impl
{
  Impl( const Pathname &pdbdir )
  {
    cache::CacheInitializer initializer(pdbdir, "zypp.db");
    if ( initializer.justInitialized() )
    {
      MIL << "database " << (pdbdir + "zypp.db") << " was just created" << endl;
    }
    
    context.reset(new DatabaseContext);
    context->dbdir = pdbdir;
    
    try
    {
      context->con.open( (pdbdir + "zypp.db").asString().c_str());
      //_insert_resolvable_cmd = new sqlite3_command( *_con, INSERT_RESOLVABLE_QUERY );                                    
      //_insert_package_cmd = new sqlite3_command( *_con, INSERT_PACKAGE_QUERY );
      context->select_vercap_cmd.reset( new sqlite3_command( context->con, "select c.refers_kind, n.name, v.version, v.release, v.epoch, v.relation from names n, capabilities c, versioned_capabilities v  where v.name_id=n.id and c.id=v.dependency_id and c.resolvable_id=:rid and c.dependency_type=:dtype;"));
    }
    catch(exception &ex)
    {
      //ZYPP_CAUGHT(ex);
      ZYPP_THROW(Exception(ex.what()));
    }
  }
  
  DatabaseContext_Ptr context;
};

CacheQuery::CacheQuery( const Pathname &dbdir )
    : _pimpl( new Impl(dbdir) )
{
}

CapabilityQuery CacheQuery::createCapabilityQuery( const zypp::Dep &deptype, const data::RecordId &resolvable_id  )
{
  return CapabilityQuery( new CapabilityQuery::Impl( _pimpl->context, deptype, resolvable_id) );
}

CacheQuery::~CacheQuery()
{
}

