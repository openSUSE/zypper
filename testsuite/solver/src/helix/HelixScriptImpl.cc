/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/solver/temporary/HelixScriptImpl.cc
 *
*/

#include "HelixScriptImpl.h"
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
//        CLASS NAME : HelixScriptImpl
//
///////////////////////////////////////////////////////////////////

/** Default ctor
*/
HelixScriptImpl::HelixScriptImpl (Source_Ref source_r, const zypp::HelixParser & parsed)
    : _source (source_r)
    , _size_installed(parsed.installedSize)
{
}

Source_Ref
HelixScriptImpl::source() const
{ return _source; }

/** Get the script to perform the change */
std::string HelixScriptImpl::do_script() const
{ return "do_script"; }

/** Get the script to undo the change */
std::string HelixScriptImpl::undo_script() const
{ return "undo_script"; }

/** Check whether script to undo the change is available */
bool HelixScriptImpl::undo_available() const
{ return false; }

ByteCount HelixScriptImpl::size() const
{ return _size_installed; }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
