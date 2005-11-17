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

#include "zypp/ResObject.h"
#include "zypp/detail/ScriptImplIf.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Script
  //
  /** Class representing an update script.
  */
  class Script : public ResObject
  {
  public:
    typedef Script                          Self;
    typedef detail::ScriptImplIf            Impl;
    typedef base::intrusive_ptr<Self>       Ptr;
    typedef base::intrusive_ptr<const Self> constPtr;

  public:
    /** Get the script to perform the change */
    std::string do_script();
    /** Get the script to undo the change */
    std::string undo_script();
    /** Check whether script to undo the change is available */
    bool undo_available();

  protected:
    /** Ctor */
    Script( const std::string & name_r,
            const Edition & edition_r,
            const Arch & arch_r );
    /** Dtor */
    virtual ~Script();

  private:
    /** Access implementation */
    virtual Impl & pimpl() = 0;
    /** Access implementation */
    virtual const Impl & pimpl() const = 0;
  };

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SCRIPT_H
