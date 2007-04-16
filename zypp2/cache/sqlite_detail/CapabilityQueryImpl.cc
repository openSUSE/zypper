/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/


#include "zypp/base/Logger.h"

#include "zypp2/cache/DatabaseTypes.h"
#include "zypp2/cache/CacheInitializer.h"
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

CapabilityQuery::Impl::Impl( DatabaseContext_Ptr p_context, const data::RecordId &resolvable_id  )
  : context(p_context), _resolvable_id(resolvable_id)
    , _vercap_read(false), _filecap_read(false)
    , _vercap_done(false), _filecap_done(false)
{}

CapabilityQuery::Impl::~Impl()
{
  //MIL << endl;
}


} // ns cache
} //ns zypp

