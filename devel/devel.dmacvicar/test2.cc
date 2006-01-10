#include "PersistentStorage.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include "dbxml/DbXml.hpp"

#include <zypp/base/Logger.h>
///////////////////////////////////////////////////////////////////

#include <zypp/Message.h>
#include <zypp/detail/MessageImpl.h>
#include <zypp/detail/PatchImpl.h>
#include <zypp/Patch.h>
#include <zypp/Package.h>
#include <zypp/Edition.h>
#include <zypp/CapSet.h>
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
using namespace zypp::storage;
//using namespace DbXml;

template<class T>
std::string toXML( T obj ); //undefined

template<> // or constPtr?
std::string toXML( const Edition edition )
{
  stringstream out;
  out << "<edition>" << edition.asString() << "</edition>";
  return out.str();
}

template<> // or constPtr?
std::string toXML( Capability cap )
{
  stringstream out;
  out << "<capability refers=\"" << cap.refers() << "\">" <<  cap.asString() << "</capability>" << std::endl;
  return out.str();
}

template<> // or constPtr?
std::string toXML( const CapSet caps )
{
  stringstream out;
  CapSet::iterator it = caps.begin();
  for ( ; it != caps.end(); ++it)
  {
    out << toXML((*it));
  }
  return out.str(); 
}

template<> // or constPtr?
std::string toXML( const Dependencies dep )
{
  stringstream out;
  out << "  <dependencies>" << std::endl;
  out << "    <provides>" << std::endl;
  out << "      " << toXML(dep.provides) << " " << std::endl;
  out << "    </provides>" << std::endl;
  out << "    <prerequires>" << std::endl;
  out << "    </prerequires>" << std::endl;
  out << "    <conflicts>" << std::endl;
  out << "    </conflicts>" << std::endl;
  out << "  </dependencies>" << std::endl;
  return out.str();
  
}

template<> // or constPtr?
std::string toXML( Resolvable::Ptr obj )
{
  stringstream out;
  out << "<resolvable-object type=\"" << obj->kind() << "\">" << std::endl;
  out << "  <name>" << obj->name() << "</name>" << std::endl;
  // is this shared? uh
  out << "  " << toXML(obj->edition()) << std::endl;
  out << "  " << toXML(obj->deps()) << std::endl;
  out << "</resolvable-object>" << std::endl;
  return out.str();
}

template<> // or constPtr?
std::string toXML( Patch::Ptr obj )
{
  stringstream out;
  return out.str();
}

class XMLBackend : public base::ReferenceCounted, private base::NonCopyable
{
	public:
	//friend std::ostream & operator<<( std::ostream & str, const PatchYUMSerializer & obj );
	typedef intrusive_ptr<XMLBackend> Ptr;
	typedef intrusive_ptr<const XMLBackend> constPtr;

  void initDatabase()
  {
    // Get a manager object.
    //XmlManager myManager;
    // Open a container
    //XmlContainer myContainer =  myManager.openContainer("zypp_db.dbxml");
  }

	XMLBackend()
	{
		
	}

  ~XMLBackend()
  {
    DBG << endl;
  }
  
  void storeObject( Resolvable::Ptr res )
  {
    DBG << std::endl;
    DBG << toXML(res) << std::endl;
  }

	private:
	//std::ostream & operator<<( std::ostream & str, const PatchYUMSerializer & obj )
};



int main()
{
	//zypp::storage::PersistentStorage ps;
	//ps.doTest();

	//INT << "===[START]==========================================" << endl;
	//RpmDb db;
	//DBG << "===[DB OBJECT CREATED]==============================" << endl;
	//db.initDatabase();
	//DBG << "===[DATABASE INITIALIZED]===========================" << endl;
	//std::list<Package::Ptr> packages = db.getPackages();
	//for (std::list<Package::Ptr>::const_iterator it = packages.begin(); it != packages.end(); it++)
	//{
	//	DBG << **it << endl;
	//}
	//INT << "===[END]============================================" << endl;

	INT << "===[START]==========================================" << endl;
	YUMSource src;
	Patch::Ptr patch1;
	
	//YUMPatchParser iter(cin,"");
	ifstream patch_file("patch.xml");
	YUMPatchParser iter(patch_file,"");
	for (; !iter.atEnd(); ++iter)
	{
		patch1 = src.createPatch(**iter);
	}
	if (iter.errorStatus())
		throw *iter.errorStatus();

	// process the patch
	
	//DBG << patch1 << endl;
	//DBG << *patch1 << endl;
	//DBG << patch1->deps() << endl;
	//Patch::AtomList at = patch1->atoms();
	//for (Patch::AtomList::iterator it = at.begin(); it != at.end(); it++)
	//{
	//	DBG << **it << endl;
	//	DBG << (**it).deps() << endl;
	//}
	//INT << "===[END]============================================" << endl;
	//PatchYUMSerializer serializer(patch1);
	//DBG << endl << serializer << endl;

  XMLBackend backend;
  backend.storeObject(patch1);
	return 1;	
}

/*
#include "DbXml.hpp"
...

using namespace DbXml;
int main(void)
{
    XmlManager myManager;   // The manager is closed when
                            // it goes out of scope.

    // Assumes the container currently exists.
    myManager.renameContainer("/export/xml/myContainer.bdbxml",
                              "/export2/xml/myContainer.bdbxml");


    myManager.removeContainer("/export2/xml/myContainer.bdbxml");

    return(0);
} 
*/

