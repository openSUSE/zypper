/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/detail/ScriptImpl.h
 *
*/
#ifndef ZYPP_DETAIL_SCRIPTIMPL_H
#define ZYPP_DETAIL_SCRIPTIMPL_H

#include "zypp/detail/ScriptImplIf.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ScriptImpl
    //
    /** Class representing an update script */
    class ScriptImpl : public ScriptImplIf
    {
    public:
      /** Default ctor */
      ScriptImpl();
      /** Dtor */
      ~ScriptImpl();

    public:
      /** Return an inlined script if available.
       * Otherwise it is available at \ref doScriptLocation.
       */
      virtual std::string doScriptInlined() const;

      /** Return an inlined undo script if available.
       * Otherwise it is available at \ref undoScriptLocation.
       */
      virtual std::string undoScriptInlined() const;

    protected:
      /** The script to perform the change */
      std::string _doScript;
      /** The script to undo the change */
      std::string _undoScript;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_SCRIPTIMPL_H
