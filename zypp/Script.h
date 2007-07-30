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
    typedef detail::ScriptImplIf     Impl;
    typedef Script                   Self;
    typedef ResTraits<Self>          TraitsType;
    typedef TraitsType::PtrType      Ptr;
    typedef TraitsType::constPtrType constPtr;

  public:
     /** Check whether a script is available. */
    bool doAvailable() const;

   /** Return an inlined script if available.
     * Otherwise it is available at \ref doScriptLocation.
     */
    std::string doScriptInlined() const;

    /** Location of the script, unless it is available inlined.
     * \see \ref doScriptInlined
     */
    OnMediaLocation doScriptLocation() const;

    /** Check whether a script to undo the change is available. */
    bool undoAvailable() const;

    /** Return an inlined undo script if available.
     * Otherwise it is available at \ref undoScriptLocation.
     */
    std::string undoScriptInlined() const;

    /** Location of the undo script, unless it is available inlined.
     * \see \ref undoScriptInlined
     */
    OnMediaLocation undoScriptLocation() const;

  protected:
    /** Ctor */
    Script( const NVRAD & nvrad_r );
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
