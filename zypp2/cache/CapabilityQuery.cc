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
#include "zypp2/cache/sqlite_detail/CacheSqlite.h"
#include "zypp2/cache/CapabilityQuery.h"
#include "zypp2/cache/sqlite_detail/CapabilityQueryImpl.h"

using namespace std;
using namespace zypp;
using namespace zypp::capability;
using namespace zypp::cache;
using namespace sqlite3x;

namespace zypp { namespace cache {

///////////////////////////////////////////////////////////////
// CAPABILITY QUERY                                         //
//////////////////////////////////////////////////////////////

CapabilityQuery::CapabilityQuery( Impl *impl)
  : _pimpl( impl)
{
     //int deptype = static_cast<int>(zypp_deptype2db_deptype(_pimpl->_deptype));
    _pimpl->context->select_versionedcap_cmd->bind(":rid", _pimpl->_resolvable_id);
    _pimpl->_versioned_reader.reset( new sqlite3_reader(_pimpl->context->select_versionedcap_cmd->executereader()));
    
    _pimpl->context->select_filecap_cmd->bind(":rid", _pimpl->_resolvable_id);
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
    
  if ( _pimpl->_filecap_read )
    return true;
  
  return false;
}

std::pair<zypp::Dep, capability::CapabilityImpl::Ptr> CapabilityQuery::value()
{
  if ( _pimpl->_vercap_read )
  {
    Resolvable::Kind refer = db_kind2zypp_kind( static_cast<db::Kind>(_pimpl->_versioned_reader->getint(0)) );
    zypp::Rel rel = db_rel2zypp_rel( static_cast<db::Rel>(_pimpl->_versioned_reader->getint(5)) );
    
    if ( rel == zypp::Rel::NONE )
    {
      capability::NamedCap *ncap = new capability::NamedCap( refer, _pimpl->_versioned_reader->getstring(1) );
      zypp::Dep deptype = db_deptype2zypp_deptype( static_cast<db::DependencyType>(_pimpl->_versioned_reader->getint(5)) );
      return make_pair( deptype, capability::NamedCap::Ptr(ncap) );
    }
    else
    {
      capability::VersionedCap *vcap = new capability::VersionedCap( refer, _pimpl->_versioned_reader->getstring(1), /* rel */ rel, Edition( _pimpl->_versioned_reader->getstring(2), _pimpl->_versioned_reader->getstring(3), _pimpl->_versioned_reader->getint(4) ) );
      zypp::Dep deptype = db_deptype2zypp_deptype( static_cast<db::DependencyType>(_pimpl->_versioned_reader->getint(5)) );
      return make_pair( deptype, capability::VersionedCap::Ptr(vcap) );
      
    }
  }
  if ( _pimpl->_filecap_read )
  {
    Resolvable::Kind refer = db_kind2zypp_kind( static_cast<db::Kind>(_pimpl->_file_reader->getint(0)) );
    capability::FileCap *fcap = new capability::FileCap( refer, _pimpl->_file_reader->getstring(1) + "/" + _pimpl->_file_reader->getstring(2) );
    zypp::Dep deptype = db_deptype2zypp_deptype( static_cast<db::DependencyType>(_pimpl->_file_reader->getint(3)) );
    return make_pair( deptype, capability::FileCap::Ptr(fcap));
  }
  ZYPP_THROW(Exception("Invalid value"));
}

} // ns cache
} //ns zypp

