/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/solver/temporary/HelixProductImpl.cc
 *
*/

#include "HelixProductImpl.h"
#include "zypp/base/String.h"
#include "zypp/base/Logger.h"

using namespace std;
using namespace zypp::detail;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
//
//        CLASS NAME : HelixProductImpl
//
///////////////////////////////////////////////////////////////////

/** Default ctor
*/
HelixProductImpl::HelixProductImpl (const zypp::HelixParser & parsed)
    : _summary(parsed.summary)
    , _description()
    , _group(parsed.section)
    , _install_only(parsed.installOnly)
    , _size_installed(parsed.installedSize)
    , _size_archive(parsed.fileSize)
{
    _description.push_back(parsed.description);
}

/** Package summary */
Label HelixProductImpl::summary() const
{ return _summary; }

/** Package description */
Text HelixProductImpl::description() const
{ return _description; }

PackageGroup HelixProductImpl::group() const
{ return _group; }

ByteCount HelixProductImpl::size() const
{ return _size_installed; }

/** */
ByteCount HelixProductImpl::archivesize() const
{ return _size_archive; }

bool HelixProductImpl::installOnly() const
{ return _install_only; }


  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
