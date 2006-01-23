/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/solver/temporary/HelixPatternImpl.cc
 *
*/

#include "HelixPatternImpl.h"
#include "zypp/base/String.h"
#include "zypp/base/Logger.h"

using namespace std;
using namespace zypp::detail;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
//
//        CLASS NAME : HelixPatternImpl
//
///////////////////////////////////////////////////////////////////

/** Default ctor
*/
HelixPatternImpl::HelixPatternImpl (const zypp::HelixParser & parsed)
    : _summary(parsed.summary)
    , _description()
    , _group(parsed.section)
    , _install_only(parsed.installOnly)
    , _size_installed(parsed.installedSize)
    , _size_archive(parsed.fileSize)
{
    _description.push_back(parsed.description);
}

/** Pattern summary */
Label HelixPatternImpl::summary() const
{ return _summary; }

/** Pattern description */
Text HelixPatternImpl::description() const
{ return _description; }

PackageGroup HelixPatternImpl::group() const
{ return _group; }

ByteCount HelixPatternImpl::size() const
{ return _size_installed; }

/** */
ByteCount HelixPatternImpl::archivesize() const
{ return _size_archive; }

bool HelixPatternImpl::installOnly() const
{ return _install_only; }


  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
