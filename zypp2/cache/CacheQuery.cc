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
  sqlite3_command_ptr select_versionedcap_cmd;
  sqlite3_command_ptr select_namedcap_cmd;
  sqlite3_command_ptr select_filecap_cmd;
  Pathname dbdir;
};

typedef shared_ptr<DatabaseContext> DatabaseContext_Ptr;

///////////////////////////////////////////////////////////////
// CAPABILITY QUERY                                         //
//////////////////////////////////////////////////////////////

class CapabilityQuery::Impl
{
  public:
  Impl( DatabaseContext_Ptr p_context, const zypp::Dep &deptype, const data::RecordId &resolvable_id  )
    : context(p_context), _deptype(deptype), _resolvable_id(resolvable_id)
      , _vercap_read(false), _namedcap_read(false), _filecap_read(false)
      , _vercap_done(false), _namedcap_done(false), _filecap_done(false)
  {}
  
  ~Impl()
  {
    MIL << endl;
  }
  
  DatabaseContext_Ptr context;
  sqlite3_reader_ptr _versioned_reader;
  sqlite3_reader_ptr _named_reader;
  sqlite3_reader_ptr _file_reader;
  zypp::Dep _deptype;
  data::RecordId _resolvable_id;
  bool _vercap_read;
  bool _namedcap_read;
  bool _filecap_read;
  
  bool _vercap_done;
  bool _namedcap_done;
  bool _filecap_done;
  
};

CapabilityQuery::CapabilityQuery( Impl *impl)
  : _pimpl( impl)
{
     int deptype = static_cast<int>(zypp_deptype2db_deptype(_pimpl->_deptype));
    _pimpl->context->select_versionedcap_cmd->bind(":rid", _pimpl->_resolvable_id);
    _pimpl->context->select_versionedcap_cmd->bind(":dtype", deptype );
    _pimpl->_versioned_reader.reset( new sqlite3_reader(_pimpl->context->select_versionedcap_cmd->executereader()));
    
    _pimpl->context->select_namedcap_cmd->bind(":rid", _pimpl->_resolvable_id);
    _pimpl->context->select_namedcap_cmd->bind(":dtype", deptype );
    _pimpl->_named_reader.reset( new sqlite3_reader(_pimpl->context->select_namedcap_cmd->executereader()));
    
    _pimpl->context->select_filecap_cmd->bind(":rid", _pimpl->_resolvable_id);
    _pimpl->context->select_filecap_cmd->bind(":dtype", deptype );
    _pimpl->_file_reader.reset( new sqlite3_reader(_pimpl->context->select_filecap_cmd->executereader()));
    
    MIL << "Done setup query" << endl;
    read();
    MIL << "Done first read" << endl;
}

CapabilityQuery::~CapabilityQuery()
{
  //MIL << endl;
  // it has to be disposed before the commands
  //_pimpl->_versioned_reader.reset();
  //_pimpl->_named_reader.reset();
  //_pimpl->_file_reader.reset();
}

bool CapabilityQuery::read()
{
  if (!_pimpl->_vercap_done)
  {
    if ( _pimpl->_vercap_read = _pimpl->_versioned_reader->read() )
      return true;
    else
      _pimpl->_vercap_done = true;
  }
        
  if (!_pimpl->_namedcap_done)
  {
    if ( _pimpl->_namedcap_read = _pimpl->_named_reader->read() )
      return true;
    else
      _pimpl->_namedcap_done = true;
  }
  
  if (!_pimpl->_filecap_done)
  {
    if ( _pimpl->_filecap_read = _pimpl->_file_reader->read() )
      return true;
    else
      _pimpl->_filecap_done = true;
  }
  
  return false;
}
  
bool CapabilityQuery::valid() const
{
 if ( _pimpl->_vercap_read )
    return true;
  
  if ( _pimpl->_namedcap_read )
    return true;
  
  if ( _pimpl->_filecap_read )
    return true;
  
  return false;
}

CapabilityImpl::Ptr CapabilityQuery::value()
{
  if ( _pimpl->_vercap_read )
  {
    Resolvable::Kind refer = db_kind2zypp_kind( static_cast<db::Kind>(_pimpl->_versioned_reader->getint(0)) );
    zypp::Rel rel = db_rel2zypp_rel( static_cast<db::Rel>(_pimpl->_versioned_reader->getint(5)) );
    capability::VersionedCap *vcap = new capability::VersionedCap( refer, _pimpl->_versioned_reader->getstring(1), /* rel */ rel, Edition( _pimpl->_versioned_reader->getstring(2), _pimpl->_versioned_reader->getstring(3), _pimpl->_versioned_reader->getint(4) ) );
    return capability::VersionedCap::Ptr(vcap);
  }
  if ( _pimpl->_namedcap_read )
  {
    Resolvable::Kind refer = db_kind2zypp_kind( static_cast<db::Kind>(_pimpl->_named_reader->getint(0)) );
    capability::NamedCap *ncap = new capability::NamedCap( refer, _pimpl->_named_reader->getstring(1) );
    return capability::NamedCap::Ptr(ncap);
  }
  if ( _pimpl->_filecap_read )
  {
    Resolvable::Kind refer = db_kind2zypp_kind( static_cast<db::Kind>(_pimpl->_file_reader->getint(0)) );
    capability::FileCap *fcap = new capability::FileCap( refer, _pimpl->_file_reader->getstring(1) + "/" + _pimpl->_file_reader->getstring(2) );
    return capability::FileCap::Ptr(fcap);
  }
  return 0L;
}

///////////////////////////////////////////////////////////////
// CACHE QUERY                                              //
//////////////////////////////////////////////////////////////

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
      
      // precompile statements
      context->select_versionedcap_cmd.reset( new sqlite3_command( context->con, "select c.refers_kind, n.name, v.version, v.release, v.epoch, v.relation from names n, capabilities c, versioned_capabilities v  where v.name_id=n.id and c.id=v.dependency_id and c.resolvable_id=:rid and c.dependency_type=:dtype;"));
      context->select_namedcap_cmd.reset( new sqlite3_command( context->con, "select c.refers_kind, n.name from names n, capabilities c, named_capabilities nc  where nc.name_id=n.id and c.id=nc.dependency_id and c.resolvable_id=:rid and c.dependency_type=:dtype;"));
      context->select_filecap_cmd.reset( new sqlite3_command( context->con, "select c.refers_kind, dn.name, fn.name  from file_names fn, dir_names dn, capabilities c, file_capabilities fc, files f  where f.id=fc.file_id and f.dir_name_id=dn.id and f.file_name_id=fn.id and c.id=fc.dependency_id and c.resolvable_id=:rid and c.dependency_type=:dtype;"));
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

