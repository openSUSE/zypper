#include <iostream>
#include <zypp/base/Logger.h>

///////////////////////////////////////////////////////////////////

#include <zypp/Message.h>
#include <zypp/detail/MessageImpl.h>
#include <zypp/detail/PatchImpl.h>
#include <zypp/Patch.h>
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
#include <zypp/source/yum/YUMSource.h>

#include <map>
#include <set>

#include <zypp/CapFactory.h>

using namespace zypp::detail;

using namespace std;
using namespace zypp;
using namespace zypp::parser::yum;
using namespace zypp::source::yum;


      // FIXME me this piece of code after solutions are selected
      // this orders the atoms of a patch
/*
      Resolvable::Ptr previous;
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
*/

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

YUMSource src;
Patch::Ptr patch1;
YUMPatchParser iter(cin,"");
for (;
     !iter.atEnd();
     ++iter) {
	patch1 = src.createPatch(**iter);
     }
if (iter.errorStatus())
  throw *iter.errorStatus();



// process the patch

DBG << patch1 << endl;
DBG << *patch1 << endl;
//DBG << patch1->deps() << endl;
Patch::AtomList at = patch1->atoms();
for (Patch::AtomList::iterator it = at.begin();
     it != at.end();
     it++)
{
  //DBG << **it << endl;
  //DBG << (**it).deps() << endl;
}

  INT << "===[END]============================================" << endl;
  return 0;
}
