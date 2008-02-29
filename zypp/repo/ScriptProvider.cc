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
#include <fstream>

#include "zypp/repo/ScriptProvider.h"
#include "zypp/PathInfo.h"
#include "zypp/TmpPath.h"
#include "zypp/Script.h"

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

        // 1st try inlined
        std::string inlined( (script_r.*inlined_r)() );
        if ( ! inlined.empty() )
        {
          // Take care the TmpFile goes out of scope BEFORE the
          // ofstream opens the file again.
          ret = ManagedFile( filesystem::TmpFile( filesystem::TmpPath::defaultLocation(),
                                                  "zypp-script-"+script_r.name() ),
                             filesystem::unlink );
          std::ofstream str( ret.value().c_str() );
          str << inlined << endl;
        }
        else
        {
          // otherwise try download
          OnMediaLocation location( (script_r.*location_r)() );
          if ( ! location.filename().empty() )
          {
            ret = access_r.provideFile( script_r.repoInfo(), location );
          }
          else
          {
            // no script
            return ManagedFile();
          }
        }

        // HERE: got the script
        filesystem::chmod( ret, 0700 );
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
    ScriptProvider::ScriptProvider( repo::RepoMediaAccess & access_r )
      : _access( access_r )
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : ScriptProvider::~ScriptProvider
    //	METHOD TYPE : Dtor
    //
    ScriptProvider::~ScriptProvider()
    {}

    ManagedFile ScriptProvider::provideDoScript( const Script_constPtr & script_r ) const
    {
      ManagedFile ret;
      if ( script_r )
      {
        return doProvideScript( _access, *script_r,
                                &Script::doScriptInlined,
                                &Script::doScriptLocation );
      }
      return ret;
    }

    ManagedFile ScriptProvider::provideUndoScript( const Script_constPtr & script_r ) const
    {
      ManagedFile ret;
      if ( script_r )
      {
        return doProvideScript( _access, *script_r,
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
