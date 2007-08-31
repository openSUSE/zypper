/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/detail/PatchImplIf.cc
 *
*/
#include <iostream>

#include "zypp/base/Easy.h"
#include "zypp/base/Logger.h"
#include "zypp/ResStore.h"
#include "zypp/CapMatchHelper.h"

#include "zypp/detail/PatchImplIf.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
    // Default implementation of PatchImplIf attributes,
    // as far as resonable.
    /////////////////////////////////////////////////////////////////

    std::string PatchImplIf::id() const
    { return std::string(); }

    Date PatchImplIf::timestamp() const
    { return Date(); }

    std::string PatchImplIf::category() const
    { return std::string(); }

    bool PatchImplIf::reboot_needed() const
    { return false; }

    bool PatchImplIf::affects_pkg_manager() const
    { return false; }

    bool PatchImplIf::interactive() const
    {
      if ( reboot_needed()
           || ! licenseToConfirm().empty() )
        {
          return true;
        }

      AtomList atoms = all_atoms();
      for ( AtomList::const_iterator it = atoms.begin(); it != atoms.end(); it++)
        {
          if (    isKind<Message>( *it )
               || ! licenseToConfirm().empty() )
            {
              return true;
            }
        }

      return false;
    }

    PatchImplIf::AtomList PatchImplIf::all_atoms() const
    {
      if ( ! _atomlist )
      {
        if ( ! hasBackRef() )
        {
          // We are not jet connected to the Resolvable that
          // contains our dependencies.
          return AtomList();
        }

        // lazy init
        _atomlist.reset( new AtomList );

        // Build the list using the repositories resolvables.
        // Installed Patches (no repository) have this method overloaded.
        if ( repository() )
        {
          const CapSet &   requires( self()->dep( Dep::REQUIRES ) );
          const ResStore & store( repository().resolvables() );

          for_( req, requires.begin(), requires.end() )
          {
            // lookup Patch requirements that refer to an Atom, Script or Message.
            if ( refersTo<Atom>( *req ) || refersTo<Script>( *req ) || refersTo<Message>( *req ) )
            {
              for_( res, store.begin(), store.end() )
              {
                // Collect ALL matches in the store.
                if ( hasMatches( (*res)->dep( Dep::PROVIDES ), (*req) ) )
                {
                  _atomlist->push_back( *res );
                }
              }
            }
          }
        }
      }
      return *_atomlist;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
