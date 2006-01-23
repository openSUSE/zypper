/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/solver/temporary/HelixPackageImpl.cc
 *
*/

#include "HelixPackageImpl.h"
#include "zypp/base/String.h"
#include "zypp/base/Logger.h"

using namespace std;
using namespace zypp::detail;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
//
//        CLASS NAME : HelixPackageImpl
//
///////////////////////////////////////////////////////////////////

/** Default ctor
*/
HelixPackageImpl::HelixPackageImpl (const zypp::HelixParser & parsed)
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
Label HelixPackageImpl::summary() const
{ return _summary; }

/** Package description */
Text HelixPackageImpl::description() const
{ return _description; }

PackageGroup HelixPackageImpl::group() const
{ return _group; }

ByteCount HelixPackageImpl::size() const
{ return _size_installed; }

/** */
ByteCount HelixPackageImpl::archivesize() const
{ return _size_archive; }

bool HelixPackageImpl::installOnly() const
{ return _install_only; }


  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
