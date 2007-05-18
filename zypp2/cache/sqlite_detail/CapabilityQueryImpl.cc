/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/


#include "zypp/base/Logger.h"

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

CapabilityQuery::Impl::Impl( DatabaseContext_Ptr p_context )
  : context(p_context)
    , _namedcap_read(false), _filecap_read(false)
    , _namedcap_done(false), _filecap_done(false)
    , _
{}

CapabilityQuery::Impl::~Impl()
{
  //MIL << endl;
}


} // ns cache
} //ns zypp

