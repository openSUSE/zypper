#include "PersistentStorage.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <streambuf>

#include "boost/filesystem/operations.hpp" // includes boost/filesystem/path.hpp
#include "boost/filesystem/fstream.hpp"    // ditto

#include <boost/iostreams/device/file_descriptor.hpp>

#include "dbxml/DbXml.hpp"

#include <zypp/base/Logger.h>
///////////////////////////////////////////////////////////////////

#include <zypp/parser/yum/YUMParser.h>
#include <zypp/base/Logger.h>
#include <zypp/source/yum/YUMScriptImpl.h>
#include <zypp/source/yum/YUMMessageImpl.h>
#include <zypp/source/yum/YUMPackageImpl.h>
#include <zypp/source/yum/YUMSourceImpl.h>

#include <map>
#include <set>

#include <zypp/CapFactory.h>

#include "serialize.h"

using namespace zypp::detail;

using namespace std;
using namespace zypp;
using namespace zypp::parser::yum;
using namespace zypp::source::yum;
using namespace zypp::storage;



//using namespace DbXml;

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
	YUMSourceImpl src;
	Patch::Ptr patch1;
	
	//YUMPatchParser iter(cin,"");
	std::ifstream patch_file("patch.xml");
	YUMPatchParser iter(patch_file,"");
	for (; !iter.atEnd(); ++iter)
	{
		patch1 = src.createPatch(**iter);
	}
	if (iter.errorStatus())
		throw *iter.errorStatus();

  PersistentStorage backend;
  backend.storeObject(patch1);

  // test xml 2 object
  std::string xml = toXML(patch1);
  std::stringstream str;
  str << xml;
  Patch::Ptr patch2;
  YUMPatchParser iter2(str,"");
  for (; !iter2.atEnd(); ++iter2)
  {
    patch2 = src.createPatch(**iter2);
  }
  cout << toXML(patch2);
  //backend.storePatch(patch1);
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

