#include <iostream>
#include <zypp/base/Logger.h>

///////////////////////////////////////////////////////////////////

#include <zypp/Message.h>
#include <zypp/detail/MessageImpl.h>
#include <zypp/detail/PatchImpl.h>
#include <zypp/Package.h>
#include <zypp/detail/PackageImpl.h>


using namespace std;
using namespace zypp;

class MyPatchImpl : public detail::PatchImpl
{
  public:
    MyPatchImpl (std::string name, std::list<ResolvablePtr> atoms)
    : PatchImpl (name,
		 Edition (),
		 Arch ("noarch"))
    {
      _reboot_needed = false;
      _atoms = atoms;
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

  atom_list atoms;

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

  MyPatchImpl q ("patch1", atoms);
  DBG << q << endl;
  DBG << "  Interactive: " << q.interactive () << endl;
  atom_list a = q.all_atoms ();
  for (atom_list::iterator it = a.begin (); it != a.end (); it++)
  {
    DBG << "  " << **it << endl;
  }

  INT << "====================================================" << endl;

  std::string _name3( "msg" );
  Arch    _arch3( "noarch" );
  detail::MessageImplPtr mi(new detail::MessageImpl(_name3,Edition (),_arch3));
  MessagePtr m (new Message (mi));
  atoms.push_back (m);

  MyPatchImpl q2 ("patch2", atoms);
  DBG << q2 << endl;
  DBG << "  Interactive: " << q2.interactive () << endl;
  a = q2.all_atoms ();
  for (atom_list::iterator it = a.begin (); it != a.end (); it++)
  {
    DBG << "  " << **it << endl;
  }

  INT << "===[END]============================================" << endl;
  return 0;
}
