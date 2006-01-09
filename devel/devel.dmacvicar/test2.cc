#include "PersistentStorage.h"

#include <iostream>
#include <fstream>

#include "dbxml/DbXml.hpp"

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
using namespace zypp::storage;
using namespace DbXml;

/*
class YUMSerializer : public base::ReferenceCounted, private base::NonCopyable
{
	public:
	friend std::ostream & operator<<( std::ostream & str, const YUMSerializer & obj );
	typedef intrusive_ptr<PatchYUMSerializer> Ptr;
	typedef intrusive_ptr<const PatchYUMSerializer> constPtr;
	YUMSerializer( Resolvable::constPtr resolvable )
	{
		m_resolvable = resolvable;
	}
	private:
	Resolvable::constPtr m_resolvable;
};
*/

class XMLBackend : public base::ReferenceCounted, private base::NonCopyable
{
	public:
	//friend std::ostream & operator<<( std::ostream & str, const PatchYUMSerializer & obj );
	typedef intrusive_ptr<XMLBackend> Ptr;
	typedef intrusive_ptr<const XMLBackend> constPtr;

  void initDatabase()
  {
    // Get a manager object.
    XmlManager myManager;
    // Open a container
    XmlContainer myContainer =  myManager.openContainer("zypp_db.dbxml");
  }

	XMLBackend()
	{
		
	}
  protected:
  const std::string serializePatch( Patch::constPtr patch ) const
  {
    std::string str;
    //ostream str(buffer);
    str += "<patch>\n";
    /*
    str << "  xmlns=\"http://novell.com/package/metadata/suse/patch\"" << endl;
    str << "  xmlns:patch=\"http://novell.com/package/metadata/suse/patch\"" << endl;
    str << "  xmlns:yum=\"http://linux.duke.edu/metadata/common\"" << endl;
    str << "  xmlns:rpm=\"http://linux.duke.edu/metadata/rpm\"" << endl;
    str << "  xmlns:suse=\"http://novell.com/package/metadata/suse/common\"" << endl;
    str << "  patchid=\"" << obj.m_patch->id() << "\"" <<  endl;
    str << "  timestamp=\"" << obj.m_patch->timestamp() << "\"" << endl;
    str << "  engine=\"1.0\">" << endl;
    str << "  <yum:name>" << obj.m_patch->name() << "</yum:name>" << endl;
    str << "  <summary lang=\"en\">"  << obj.m_patch->summary() << "</summary>" << endl;
    */
    str += "</patch>\n";
    return str;
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
	
	DBG << patch1 << endl;
	DBG << *patch1 << endl;
	DBG << patch1->deps() << endl;
	Patch::AtomList at = patch1->atoms();
	for (Patch::AtomList::iterator it = at.begin(); it != at.end(); it++)
	{
		DBG << **it << endl;
		DBG << (**it).deps() << endl;
	}
	INT << "===[END]============================================" << endl;
	//PatchYUMSerializer serializer(patch1);
	//DBG << endl << serializer << endl;
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

