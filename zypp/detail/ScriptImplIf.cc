/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/detail/ScriptImplIf.cc
 *
*/
#include "zypp/detail/ScriptImplIf.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
    // Default implementation of ScriptImplIf attributes,
    // as far as resonable.
    /////////////////////////////////////////////////////////////////

    bool ScriptImplIf::doAvailable() const
    { return ! ( doScriptInlined().empty()
                 && doScriptLocation().filename().empty() ); }

    unsigned ScriptImplIf::mediaNr() const
    { return( doScriptInlined().empty() ? doScriptLocation().medianr() : 0 ); }

    std::string ScriptImplIf::doScriptInlined() const
    { return std::string(); }

    OnMediaLocation ScriptImplIf::doScriptLocation() const
    { return OnMediaLocation(); }

    bool ScriptImplIf::undoAvailable() const
    { return ! ( undoScriptInlined().empty()
                 && undoScriptLocation().filename().empty() ); }

    std::string ScriptImplIf::undoScriptInlined() const
    { return std::string(); }

    OnMediaLocation ScriptImplIf::undoScriptLocation() const
    { return OnMediaLocation(); }

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
