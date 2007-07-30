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
#warning IMPLEMENT IT
///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace repo
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ScriptProvider::Impl
    //
    /** ScriptProvider implementation. */
    struct ScriptProvider::Impl
    {
    };
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ScriptProvider
    //
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : ScriptProvider::ScriptProvider
    //	METHOD TYPE : Ctor
    //
    ScriptProvider::ScriptProvider( repo::RepoMediaAccess & access_r,
                                    const Script::constPtr & script_r )
    //: _pimpl(  )
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
      return ManagedFile();
    }

    ManagedFile ScriptProvider::provideUndoScript() const
    {
      return ManagedFile();
    }

    /////////////////////////////////////////////////////////////////
  } // namespace repo
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
