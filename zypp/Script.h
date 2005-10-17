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

#include "zypp/detail/ScriptImpl.h"
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
  /** */
  class Script : public Resolvable
  {
  public:
    /** Default ctor */
    Script( detail::ScriptImplPtr impl_r );
    /** Dtor */
    ~Script();
  public:
    std::string do_script ();
    std::string undo_script ();
    bool undo_available ();
  private:
    /** Pointer to implementation */
    detail::ScriptImplPtr _pimpl;
  };

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SCRIPT_H
