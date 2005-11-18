#include <iostream>
#include <zypp/base/Logger.h>

///////////////////////////////////////////////////////////////////

#include <zypp/Message.h>
#include <zypp/detail/MessageImpl.h>
#include <zypp/detail/PatchImpl.h>
#include <zypp/Package.h>
#include <zypp/detail/PackageImpl.h>
#include <zypp/Script.h>
#include <zypp/detail/ScriptImpl.h>
#include <zypp/Resolvable.h>
#include <zypp/detail/ResolvableImpl.h>
#include <zypp/Capability.h>
#include <zypp/capability/CapabilityImpl.h>

#include <zypp/parser/yum/YUMParser.h>
#include <zypp/base/Logger.h>
#include <zypp/source/yum/YUMScriptImpl.h>
#include <zypp/source/yum/YUMMessageImpl.h>
#include <zypp/source/yum/YUMPackageImpl.h>


#include <map>
#include <set>

#include <zypp/CapFactory.h>

using namespace zypp::detail;

using namespace std;
using namespace zypp;
using namespace zypp::parser::YUM;
using namespace zypp::source::YUM;


CapFactory _f;

void AddDependency (detail::ResolvableImplPtr res, Capability& cap) {
  Dependencies deps = res->deps();
  CapSet req = deps.requires();
  req.insert( cap );
  deps.setRequires( req );
  res->setDeps( deps );
}

void AddAllRequires( detail::ResolvableImplPtr res, list<Capability> & caps) {
    for (list<Capability>::iterator it = caps.begin();
         it != caps.end();
	 it++)
    {
      AddDependency( res, *it );
    }
}

DEFINE_PTR_TYPE(MyPatchImpl)

class MyPatchImpl : public detail::PatchImpl
{
  public:
    MyPatchImpl( YUMPatchData & p )
    : PatchImpl (p.name,
		 Edition (),
		 Arch ("noarch"))
    {
      // Create a requires capability to the patch itself
      Capability cap( _f.parse( p.name, ResKind( "patch" )));

      // Process atoms
      detail::PatchImpl::AtomList atoms;

      for (std::list<YUMPatchAtom>::iterator it = p.atoms.begin();
	it != p.atoms.end();
	it++)
      {
	if (it->type == "script")
	{
	  ScriptImplPtr impl = new YUMScriptImpl( *it->script );
	  AddDependency(impl, cap);
	  ScriptPtr script = new Script(impl);
	  cout << *script << endl;
	  cout << script->deps() << endl;
	  atoms.push_back(script);
	}
	else if (it->type == "message")
	{
	  MessageImplPtr impl = new YUMMessageImpl( *it->message );
	  AddDependency(impl, cap);
	  MessagePtr message = new Message(impl);
	  cout << *message << endl;
	  cout << message->deps() << endl;
	  atoms.push_back(message);
	}
	else if (it->type == "package")
	{
	  PackageImplPtr impl = new YUMPackageImpl( *it->package );
	  AddDependency(impl, cap);
	  PackagePtr package = new Package(impl);
	  cout << *package << endl;
	  cout << package->deps() << endl;
	  atoms.push_back(package);
	}
      }

      // FIXME mve this piece of code after solutions are selected
      // this orders the atoms of a patch
      ResolvablePtr previous;
      bool first = true;
      for (detail::PatchImpl::AtomList::iterator it = atoms.begin();
           it != atoms.end();
	   it++)
      {
	if (! first)
	{
	  Dependencies deps = (*it)->deps();
	  CapSet req = deps.prerequires();
	  req.insert( Capability( _f.parse( previous->name(), previous->kind())));
	  deps.setPrerequires( req );
	  (*it)->setDeps( deps );
	}
	first = false;
	previous = *it;
      }

      _reboot_needed = false; // FIXME
      _atoms = atoms;
      _category = "recommended"; // FIXME
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

PatchPtr patch1;
YUMPatchParser iter(cin,"");
for (;
     !iter.atEnd();
     ++iter) {
       MyPatchImplPtr q(new MyPatchImpl(**iter));
	patch1 = new Patch (q);
     }
if (iter.errorStatus())
  throw *iter.errorStatus();



// process the patch

DBG << patch1 << endl;
DBG << *patch1 << endl;
Patch::AtomList at = patch1->atoms();
for (Patch::AtomList::iterator it = at.begin();
     it != at.end();
     it++)
{
  DBG << **it << endl;
  DBG << (**it).deps() << endl;
}

  INT << "===[END]============================================" << endl;
  return 0;
}
