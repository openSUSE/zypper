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
//        CLASS NAME : HelixProductImpl
//
///////////////////////////////////////////////////////////////////

/** Default ctor
*/
HelixProductImpl::HelixProductImpl (Source_Ref source_r, const zypp::HelixParser & parsed)
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
HelixProductImpl::source() const
{ return _source; }

/** Package summary */
TranslatedText HelixProductImpl::summary() const
{ return _summary; }

/** Package description */
TranslatedText HelixProductImpl::description() const
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
