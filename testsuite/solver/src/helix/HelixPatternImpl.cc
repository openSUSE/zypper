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
#include "zypp/source/SourceImpl.h"
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
HelixPatternImpl::HelixPatternImpl (Source_Ref source_r, const zypp::HelixParser & parsed)
    : _source (source_r)
    , _summary(parsed.summary)
    , _description(parsed.description)
    , _group(parsed.section)
    , _install_only(parsed.installOnly)
    , _size_installed(parsed.installedSize)
    , _size_archive(parsed.fileSize)
{
}

Source_Ref
HelixPatternImpl::source() const
{ return _source; }

/** Pattern summary */
const TranslatedText & HelixPatternImpl::summary() const
{ return _summary; }

/** Pattern description */
const TranslatedText & HelixPatternImpl::description() const
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
