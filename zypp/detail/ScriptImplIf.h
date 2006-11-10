/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/detail/ScriptImplIf.h
 *
*/
#ifndef ZYPP_DETAIL_SCRIPTIMPLIF_H
#define ZYPP_DETAIL_SCRIPTIMPLIF_H

#include "zypp/detail/ResObjectImplIf.h"
#include "zypp/Pathname.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class Script;

  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ScriptImplIf
    //
    /** Abstract Script implementation interface.
    */
    class ScriptImplIf : public ResObjectImplIf
    {
    public:
      typedef Script ResType;

    public:
      /** Get the script to perform the change */
      virtual Pathname do_script() const PURE_VIRTUAL;
      /** Get the script to undo the change */
      virtual Pathname undo_script() const PURE_VIRTUAL;
      /** Check whether script to undo the change is available */
      virtual bool undo_available() const PURE_VIRTUAL;
      /** */
      virtual ByteCount size() const;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_SCRIPTIMPLIF_H
