#include <iostream>
#include <fstream>
#include <sstream>
#include <streambuf>
 #include <ctime>

#include "boost/filesystem/operations.hpp" // includes boost/filesystem/path.hpp
#include "boost/filesystem/fstream.hpp"    // ditto

#include <boost/iostreams/device/file_descriptor.hpp>

#include <zypp/SourceFactory.h>

#include <zypp/base/Logger.h>
///////////////////////////////////////////////////////////////////

#include <zypp/target/store/PersistentStorage.h>
#include <zypp/target/store/XMLFilesBackend.h>

#include <zypp/parser/yum/YUMParser.h>
#include <zypp/base/Logger.h>
#include <zypp/source/yum/YUMScriptImpl.h>
#include <zypp/source/yum/YUMMessageImpl.h>
#include <zypp/source/yum/YUMPackageImpl.h>
#include <zypp/source/yum/YUMSourceImpl.h>

#include <map>
#include <set>

#include <zypp/CapFactory.h>

#include <zypp/target/store/serialize.h>

using namespace zypp::detail;

using namespace std;
using namespace zypp;
using namespace zypp::parser::yum;
using namespace zypp::source::yum;
using namespace zypp::storage;

#define PATCH_FILE "../../../devel/devel.jsrain/repodata/patch.xml"

//using namespace DbXml;

int main()
{
	INT << "===[START]==========================================" << endl;

	YUMSourceImpl srcimpl;

    Source::Impl_Ptr impl (&srcimpl );
    SourceFactory _f;
    Source src = _f.createFrom( impl );

	Patch::Ptr patch1;
	
	//YUMPatchParser iter(cin,"");
	std::ifstream patch_file(PATCH_FILE);
	YUMPatchParser iter(patch_file,"");
	for (; !iter.atEnd(); ++iter)
	{
		patch1 = srcimpl.createPatch(src, **iter);
	}
	if (iter.errorStatus())
		throw *iter.errorStatus();

  XMLFilesBackend backend;
  backend.setRandomFileNameEnabled(true);
  
  clock_t time_start, curr_time;
  time_start = clock();
  int i = 0;
  DBG << "Writing Patches..." << std::endl;
  for (; i < 1000; i++)
    backend.storeObject(patch1);
  
  curr_time = clock() - time_start;           // time in micro seconds 
  DBG << "Wrote " << i << " patches in " << (double) curr_time / CLOCKS_PER_SEC << " seconds" << std::endl;

  time_start = clock();
  std::list<Resolvable::Ptr> objs = backend.storedObjects();
  curr_time = clock() - time_start;           // time in micro seconds 
  DBG << "Read " << objs.size() << " patches in " << (double) curr_time / CLOCKS_PER_SEC << " seconds" << std::endl;
  return 0;	
}
