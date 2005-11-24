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
    /** Abstact Script implementation interface.
    */
    class ScriptImplIf : public ResObjectImplIf
    {
    public:
      typedef Script ResType;

    public:
      /** Get the script to perform the change */
      virtual std::string do_script() const = 0;
      /** Get the script to undo the change */
      virtual std::string undo_script() const = 0;
      /** Check whether script to undo the change is available */
      virtual bool undo_available() const = 0;
      /** */
      virtual FSize size() const;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_SCRIPTIMPLIF_H
