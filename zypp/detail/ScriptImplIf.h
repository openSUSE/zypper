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
      /** Overloaded ResObjectImpl attribute.
       * \return The \ref doScriptLocation media number
       * or \c 0 if do-script is inlined.
       */
      virtual unsigned mediaNr() const;

    public:
       /** Check whether a script is available. */
      virtual bool doAvailable() const;

     /** Return an inlined script if available.
       * Otherwise it is available at \ref doScriptLocation.
       */
      virtual std::string doScriptInlined() const;

      /** Location of the script, unless it is available inlined.
       * \see \ref doScriptInlined
       */
      virtual OnMediaLocation doScriptLocation() const;

      /** Check whether a script to undo the change is available. */
      virtual bool undoAvailable() const;

      /** Return an inlined undo script if available.
       * Otherwise it is available at \ref undoScriptLocation.
       */
      virtual std::string undoScriptInlined() const;

      /** Location of the undo script, unless it is available inlined.
       * \see \ref undoScriptInlined
       */
      virtual OnMediaLocation undoScriptLocation() const;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_SCRIPTIMPLIF_H
