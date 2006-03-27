/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/detail/XMLScriptImpl.h
 *
*/
#ifndef ZYPP_STORE_XMLSCRIPTIMPL_H
#define ZYPP_STORE_XMLSCRIPTIMPL_H

#include "zypp/TmpPath.h"
#include "zypp/detail/ScriptImplIf.h"

using namespace zypp::filesystem;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace storage
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : XMLScriptImpl
    //
    /** Class representing an update script */
    struct XMLScriptImpl : public zypp::detail::ScriptImplIf
    {
      /** Default ctor */
      XMLScriptImpl();
      /** Dtor */
      ~XMLScriptImpl();

      /** Get the script to perform the change */
      Pathname do_script() const;
      /** Get the script to undo the change */
      Pathname undo_script() const;
      /** Check whether script to undo the change is available */
      virtual bool undo_available() const;
      
      TmpFile _do_script;
      TmpFile _undo_script;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_SCRIPTIMPL_H
