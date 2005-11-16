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


#include <map>
#include <set>

#include <zypp/CapFactory.h>

using namespace zypp::detail;

using namespace std;
using namespace zypp;
using namespace zypp::parser::YUM;
using namespace zypp::source::YUM;


CapFactory _f;

DEFINE_PTR_TYPE(MyMessageImpl)
class MyMessageImpl : public detail::MessageImpl
{
  public:
    MyMessageImpl (string name, string type, std::string text)
    : MessageImpl (name,
		   Edition (),
		   Arch ("noarch"))
    {
      _text = text;
      _type = type;
    }
};
IMPL_PTR_TYPE(MyMessageImpl)

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
      atom_list atoms;

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
      }

/*      for (list<PACKAGE>::iterator it = p.pack.begin();
           it != p.pack.end();
           it++)
      {
	detail::PackageImplPtr pi( new detail::PackageImpl(
	  it->name,
	  Edition( it->version, it->release ),
	  Arch( it->arch )));
	AddDependency( pi, cap );
	AddAllRequires( pi, it->requires );
	PackagePtr p( new Package( pi ));
	DBG << *p << endl;
	DBG << p->deps() << endl;
	atoms.push_back( p );
      }
      for (list<MESSAGE>::iterator it = p.msg.begin();
           it != p.msg.end();
           it++)
      {
	detail::MessageImplPtr pi( new MyMessageImpl(
	  it->name,
	  it->type,
	  it->text));
	AddDependency( pi, cap );
	MessagePtr p( new Message( pi ));
	DBG << *p << endl;
	DBG << p->deps() << endl;
	atoms.push_back( p );
      }
*/
      // FIXME mve this piece of code after solutions are selected
      // this orders the atoms of a patch
      ResolvablePtr previous;
      bool first = true;
      for (atom_list::iterator it = atoms.begin();
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

// filling structures
/*
PACKAGE foo;
foo.name = "foo";
foo.version = "3.0";
foo.release = "5";
foo.arch = "noarch";

PACKAGE foocomp;
foocomp.name = "foo-compat";
foocomp.version = "3.0";
foocomp.release = "5";
foocomp.arch = "noarch";
foocomp.requires.push_back( Capability( _f.parse( "foo", ResKind( "package" ))));

PACKAGE bar;
bar.name = "bar";
bar.version = "2.8";
bar.release = "2";
bar.arch = "noarch";

MESSAGE msg;
msg.name = "msg";
msg.type = "OK";
msg.text = "Hello World";

SCRIPT scr;
scr.name = "scr";
scr.do_script = "/bin/bash";
scr.undo_script = "/bin/unbash";

list<PACKAGE> pkgs;
list<MESSAGE> msgs;
list<SCRIPT> scrs;

pkgs.push_back( foo );
pkgs.push_back( foocomp );
pkgs.push_back( bar );
msgs.push_back( msg );
scrs.push_back( scr );

PATCH ptch;
ptch.name = "patch";
ptch.pack = pkgs;
ptch.msg = msgs;
ptch.scr = scrs;
*/
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
atom_list at = patch1->atoms();
for (atom_list::iterator it = at.begin();
     it != at.end();
     it++)
{
  DBG << **it << endl;
  DBG << (**it).deps() << endl;
}

  INT << "===[END]============================================" << endl;
  return 0;
}
