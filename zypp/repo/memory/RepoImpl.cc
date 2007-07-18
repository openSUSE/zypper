/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include <iostream>
#include <map>

#include "zypp/base/Logger.h"
#include "zypp/base/Measure.h"
#include "zypp/capability/Capabilities.h"
#include "zypp/cache/ResolvableQuery.h"
#include "zypp/detail/ResImplTraits.h"
#include "zypp/CapFactory.h"

#include "zypp/Package.h"
#include "zypp/Pattern.h"
#include "zypp/Patch.h"
#include "zypp/Message.h"
#include "zypp/Script.h"
#include "zypp/Atom.h"

#include "zypp/repo/memory/RepoImpl.h"

#include "zypp/repo/memory/PackageImpl.h"
#include "zypp/repo/memory/PatternImpl.h"
#include "zypp/repo/memory/PatchImpl.h"
#include "zypp/repo/memory/MessageImpl.h"
#include "zypp/repo/memory/ScriptImpl.h"
#include "zypp/repo/memory/AtomImpl.h"

using namespace zypp::detail;
using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace repo
{ /////////////////////////////////////////////////////////////////
 ///////////////////////////////////////////////////////////////////
namespace memory
{ /////////////////////////////////////////////////////////////////

RepoImpl::RepoImpl( const RepoInfo &repoinfo )
  : RepositoryImpl(repoinfo)
{
}

RepoImpl::~RepoImpl()
{
}

void RepoImpl::createResolvables()
{
}

void RepoImpl::createPatchAndDeltas()
{
}

/////////////////////////////////////////////////////////////////
} // namespace memory
///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
} // namespace repository
///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

