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

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Script
  //
  /** Class representing an update script.
    * \deprecated class is obsolete
 */
  class Script : public ResObject
  {
  public:
    typedef Script                   Self;
    typedef ResTraits<Self>          TraitsType;
    typedef TraitsType::PtrType      Ptr;
    typedef TraitsType::constPtrType constPtr;

  public:
     /** Check whether a script is available. */
    bool doAvailable() const ZYPP_DEPRECATED;

   /** Return an inlined script if available.
     * Otherwise it is available at \ref doScriptLocation.
     */
    std::string doScriptInlined() const ZYPP_DEPRECATED;

    /** Location of the script, unless it is available inlined.
     * \see \ref doScriptInlined
     */
    OnMediaLocation doScriptLocation() const ZYPP_DEPRECATED;

    /** Check whether a script to undo the change is available. */
    bool undoAvailable() const;

    /** Return an inlined undo script if available.
     * Otherwise it is available at \ref undoScriptLocation.
     */
    std::string undoScriptInlined() const ZYPP_DEPRECATED;

    /** Location of the undo script, unless it is available inlined.
     * \see \ref undoScriptInlined
     */
    OnMediaLocation undoScriptLocation() const ZYPP_DEPRECATED;

  protected:
    friend Ptr make<Self>( const sat::Solvable & solvable_r );
    /** Ctor */
    Script( const sat::Solvable & solvable_r ) ZYPP_DEPRECATED;
    /** Dtor */
    virtual ~Script();
  };

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SCRIPT_H
