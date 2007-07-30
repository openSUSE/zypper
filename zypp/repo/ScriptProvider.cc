/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/repo/ScriptProvider.cc
 *
*/
#include <iostream>
//#include "zypp/base/Logger.h"

#include "zypp/repo/ScriptProvider.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace repo
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    namespace
    { /////////////////////////////////////////////////////////////////

      typedef std::string     (Script::*inlined)() const;
      typedef OnMediaLocation (Script::*location)() const;

      /** Provide a Script in a local file. */
      ManagedFile doProvideScript( repo::RepoMediaAccess & access_r,
                                   const Script & script_r,
                                   inlined inlined_r, location location_r )
      {
        ManagedFile ret;
        return ret;
      }

      /////////////////////////////////////////////////////////////////
    } // namespace
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : ScriptProvider::ScriptProvider
    //	METHOD TYPE : Ctor
    //
    ScriptProvider::ScriptProvider( repo::RepoMediaAccess & access_r,
                                    const Script::constPtr & script_r )
      : _access( access_r )
      , _script( script_r )
    {
      //       ManagedFile provideFile( Repository repo_r,
      //                                const OnMediaLocation & loc_r,
      //                                const ProvideFilePolicy & policy_r = ProvideFilePolicy() );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : ScriptProvider::~ScriptProvider
    //	METHOD TYPE : Dtor
    //
    ScriptProvider::~ScriptProvider()
    {}

    ManagedFile ScriptProvider::provideDoScript() const
    {
      ManagedFile ret;
      if ( _script )
      {
        return doProvideScript( _access, *_script,
                                &Script::doScriptInlined,
                                &Script::doScriptLocation );
      }
      return ret;
    }

    ManagedFile ScriptProvider::provideUndoScript() const
    {
      ManagedFile ret;
      if ( _script )
      {
        return doProvideScript( _access, *_script,
                                &Script::undoScriptInlined,
                                &Script::undoScriptLocation );
      }
      return ret;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace repo
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
