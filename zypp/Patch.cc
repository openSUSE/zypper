/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/Patch.cc
 *
*/
#include "zypp/Patch.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  IMPL_PTR_TYPE( Patch );

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Patch::Patch
  //	METHOD TYPE : Ctor
  //
  Patch::Patch( const sat::Solvable & solvable_r )
  : ResObject( solvable_r )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Patch::~Patch
  //	METHOD TYPE : Dtor
  //
  Patch::~Patch()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	Patch interface forwarded to implementation
  //
  ///////////////////////////////////////////////////////////////////

  std::string Patch::id() const
  { return std::string(); }

  Date Patch::timestamp() const
  { return Date(); }

  std::string Patch::category() const
  { return std::string(); }

  bool Patch::reboot_needed() const
  { return false; }

  bool Patch::affects_pkg_manager() const
  { return false; }

#warning Implement PATCH::ATOMS
#if 0
  Patch::AtomList Patch::atoms() const
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
    return AtomList();
  }
#endif

  bool Patch::interactive() const
  {
#warning Implement PATCH::INTERACTIVE
#if 0
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
#endif
    return false;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
