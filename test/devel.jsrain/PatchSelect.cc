#include <iostream>
#include <zypp/base/Logger.h>

///////////////////////////////////////////////////////////////////

#include <zypp/Message.h>
#include <zypp/detail/MessageImpl.h>
#include <zypp/detail/PatchImpl.h>
#include <zypp/Package.h>
#include <zypp/detail/PackageImpl.h>

#include <map>
#include <set>


using namespace std;
using namespace zypp;

DEFINE_PTR_TYPE(MyPatchImpl)

class MyPatchImpl : public detail::PatchImpl
{
  public:
    MyPatchImpl( std::string name,
		 std::list<ResolvablePtr> atoms,
		 std::string category)
    : PatchImpl (name,
		 Edition (),
		 Arch ("noarch"))
    {
      _reboot_needed = false;
      _atoms = atoms;
      _category = category;
    }
};

IMPL_PTR_TYPE(MyPatchImpl)


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

  PatchImpl::AtomList atoms;

  std::string _name( "foo" );
  Edition _edition( "1.0", "42" );
  Arch    _arch( "i386" );
  detail::PackageImplPtr pi( new detail::PackageImpl(_name,_edition,_arch) );
  PackagePtr p (new Package (pi));
  atoms.push_back (p);

  std::string _name2( "bar" );
  Edition _edition2( "1.0", "43" );
  Arch    _arch2( "noarch" );
  detail::PackageImplPtr pi2(new detail::PackageImpl(_name2,_edition2,_arch2));
  PackagePtr p2 (new Package (pi2));
  atoms.push_back (p2);

  MyPatchImplPtr q (new MyPatchImpl ("patch1", atoms, "recommended"));
  PatchPtr patch1 (new Patch (q));

  PatchImpl::AtomList atoms2;
  std::string _name3( "msg" );
  Arch    _arch3( "noarch" );
  detail::MessageImplPtr mi(new detail::MessageImpl(_name3,Edition (),_arch3));
  MessagePtr m (new Message (mi));
  atoms2.push_back (m);

  MyPatchImplPtr q2 (new MyPatchImpl ("patch2", atoms2, "recommended"));
  PatchPtr patch2 (new Patch (q2)); 

  list<PatchPtr> patches;
  patches.push_back (patch1);
  patches.push_back (patch2);

  // input values
  bool interactive_run = true;
  set<string> levels_to_preselect;
  levels_to_preselect.insert ("security");
  levels_to_preselect.insert ("recommended");

  // output values
  list<PatchPtr> patches_to_display;
  list<PatchPtr> patches_for_manual;

// really start here

  // count patches to offer
  if (interactive_run)
  {
    for (list<PatchPtr>::iterator it = patches.begin();
         it != patches.end();
         it++)
    {
      (*it)->select ();
    }
    DBG << "Running solver here..." << endl; // TODO
    for (list<PatchPtr>::iterator it = patches.begin();
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
  for (list<PatchPtr>::iterator it = patches.begin();
       it != patches.end();
       it++)
  {
    PatchPtr ptr = *it;
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

  for (list<PatchPtr>::iterator it = patches.begin();
       it != patches.end();
       it++)
  {
    PatchPtr ptr = *it;
    if (! ptr->any_atom_selected())
    {
      ptr->select(); // TODO use deselected here instead
    }
  }

  INT << "===[END]============================================" << endl;
  return 0;
}
