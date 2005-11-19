#include <iostream>
#include <zypp/base/Logger.h>

///////////////////////////////////////////////////////////////////

#include <zypp/Message.h>
#include <zypp/detail/MessageImpl.h>
#include <zypp/detail/PatchImpl.h>
#include <zypp/Patch.h>
#include <zypp/Package.h>
#include <zypp/detail/PackageImpl.h>

#include <map>
#include <set>


using namespace std;
using namespace zypp;

class MyPatchImpl : public detail::PatchImpl
{
  public:
    MyPatchImpl( detail::PatchImpl::AtomList atoms,
		 std::string category)
    {
      _reboot_needed = false;
      _atoms = atoms;
      _category = category;
    }
};


/******************************************************************
**
**
**	FUNCTION NAME : main
**	FUNCTION TYPE : int
**
**	DESCRIPTION :
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;

  detail::PatchImpl::AtomList atoms;

  std::string _name( "foo" );
  Edition _edition( "1.0", "42" );
  Arch    _arch( "i386" );
  base::shared_ptr<detail::PackageImpl> pi( new detail::PackageImpl );
  Package::Ptr p( detail::makeResolvableFromImpl( _name, _edition, _arch, pi ));
  atoms.push_back (p);

  std::string _name2( "bar" );
  Edition _edition2( "1.0", "43" );
  Arch    _arch2( "noarch" );
  base::shared_ptr<detail::PackageImpl> pi2( new detail::PackageImpl );
  Package::Ptr p2( detail::makeResolvableFromImpl( _name2, _edition2, _arch2, pi2 ));
  atoms.push_back (p2);

  base::shared_ptr<detail::PatchImpl> pt1(new MyPatchImpl(atoms,"recommended"));
  Patch::Ptr patch1( detail::makeResolvableFromImpl( std::string("patch1"), _edition, _arch, pt1));

  detail::PatchImpl::AtomList atoms2;
  std::string _name3( "msg" );
  Arch    _arch3( "noarch" );
  base::shared_ptr<detail::MessageImpl> mi( new detail::MessageImpl );
  Message::Ptr m( detail::makeResolvableFromImpl( _name, _edition, _arch, mi ));
  atoms.push_back (m);

  base::shared_ptr<detail::PatchImpl> pt2(new MyPatchImpl(atoms,"recommended"));
  Patch::Ptr patch2( detail::makeResolvableFromImpl( std::string("patch2"), _edition, _arch, pt2));


  list<Patch::Ptr> patches;
  patches.push_back (patch1);
  patches.push_back (patch2);

  // input values
  bool interactive_run = true;
  set<string> levels_to_preselect;
  levels_to_preselect.insert ("security");
  levels_to_preselect.insert ("recommended");

  // output values
  list<Patch::Ptr> patches_to_display;
  list<Patch::Ptr> patches_for_manual;

// really start here

  // count patches to offer
  if (interactive_run)
  {
    for (list<Patch::Ptr>::iterator it = patches.begin();
         it != patches.end();
         it++)
    {
      (*it)->select ();
    }
    DBG << "Running solver here..." << endl; // TODO
    for (list<Patch::Ptr>::iterator it = patches.begin();
         it != patches.end();
         it++)
    {
      if ((*it)->any_atom_selected())
      {
	patches_to_display.push_back( *it );
      }
      (*it)->select();
    }
  }

  // count patches to preselect
  for (list<Patch::Ptr>::iterator it = patches.begin();
       it != patches.end();
       it++)
  {
    Patch::Ptr ptr = *it;
    string category = ptr->category ();
    // interactive patches can be applied only in interactive mode
    if (interactive_run
        || (! (ptr->reboot_needed() || ptr->interactive())))
    {
      if (levels_to_preselect.count( category ) > 0)
      {
	// select only packages severe enough
	DBG << "Selecting patch: " << *ptr << endl;
	ptr->select();
      }
      else
      {
	// and the rest keep neutral (they can be selected by dependencies)
	DBG << "Category unsufficient: " << *ptr << endl;
	ptr->select(); // FIXME unselect, but not taboo
      }
    }
    else
    {
      // in non-interactive mode, they mustn't be applied in any case
      DBG << "Patch interactive: " << *ptr << endl;
      ptr->select(); // FIXME taboo - mustn't be  selected by resolver
      // instead, admin should be notified
      patches_for_manual.push_back( ptr );
      
    }
  }
  DBG << "Running solver here..." << endl; // TODO
  // hoping resolver solves obsoletes as well

  for (list<Patch::Ptr>::iterator it = patches.begin();
       it != patches.end();
       it++)
  {
    Patch::Ptr ptr = *it;
    if (! ptr->any_atom_selected())
    {
      ptr->select(); // TODO use deselected here instead
    }
  }

  INT << "===[END]============================================" << endl;
  return 0;
}
