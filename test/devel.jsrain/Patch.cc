#include <iostream>
#include <zypp/base/Logger.h>

///////////////////////////////////////////////////////////////////

#include <zypp/Message.h>
#include <zypp/detail/MessageImpl.h>
#include <zypp/detail/PatchImpl.h>
#include <zypp/Package.h>
#include <zypp/Patch.h>
#include <zypp/detail/PackageImpl.h>


using namespace std;
using namespace zypp;

class MyPatchImpl : public detail::PatchImpl
{
  public:
    MyPatchImpl (detail::PatchImpl::AtomList atoms)
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
  base::shared_ptr<detail::PackageImpl> pi2(new detail::PackageImpl);
  Package::Ptr p2( detail::makeResolvableFromImpl( _name, _edition, _arch, pi2));
  atoms.push_back (p2);

  base::shared_ptr<detail::PatchImpl> pt1(new MyPatchImpl(atoms));
  Patch::Ptr q( detail::makeResolvableFromImpl( std::string("patch1"), _edition, _arch, pt1));
  DBG << *q << endl;
  DBG << "  Interactive: " << q->interactive () << endl;
  Patch::AtomList a = q->atoms ();
  for (Patch::AtomList::iterator it = a.begin (); it != a.end (); it++)
  {
    DBG << "  " << **it << endl;
  }

  INT << "====================================================" << endl;

  std::string _name3( "msg" );
  Edition _ed3( "1.0", "42" );
  Arch    _arch3( "noarch" );
  base::shared_ptr<detail::MessageImpl> mi(new detail::MessageImpl);
  Message::Ptr m (detail::makeResolvableFromImpl( _name3, _ed3, _arch3, mi));

  atoms.push_back (m);

  base::shared_ptr<detail::PatchImpl> pt2(new MyPatchImpl(atoms));
  Patch::Ptr q2( detail::makeResolvableFromImpl( std::string("patch2"), _edition, _arch, pt2));
  DBG << *q2 << endl;
  DBG << "  Interactive: " << q2->interactive () << endl;
  a = q2->atoms ();
  for (Patch::AtomList::iterator it = a.begin (); it != a.end (); it++)
  {
    DBG << "  " << **it << endl;
  }

  INT << "===[END]============================================" << endl;
  return 0;
}
