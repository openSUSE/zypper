/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/Script.h
 *
*/
#ifndef ZYPP_SCRIPT_H
#define ZYPP_SCRIPT_H

#include <iosfwd>

#include "zypp/Resolvable.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////
    DEFINE_PTR_TYPE(ScriptImpl)
    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  DEFINE_PTR_TYPE(Script)

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Script
  //
  /** Class representing an update script */
  class Script : public Resolvable
  {
  public:
    /** Default ctor */
    Script( detail::ScriptImplPtr impl_r );
    /** Dtor */
    ~Script();
  public:
    /** Get the script to perform the change */
    std::string do_script ();
    /** Get the script to undo the change */
    std::string undo_script ();
    /** Check whether script to undo the change is available */
    bool undo_available ();
  private:
    /** Pointer to implementation */
    detail::ScriptImplPtr _pimpl;
  };

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SCRIPT_H
