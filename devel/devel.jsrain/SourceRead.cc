#include <iostream>
#include <fstream>
#include <zypp/base/Logger.h>
#include <zypp/detail/MessageImpl.h>
#include <zypp/Message.h>
#include <YUMParser.h>
#include <zypp/base/Logger.h>
#include <map>
#include "zypp/source/yum/YUMSource.h"


using namespace std;
using namespace zypp;
using namespace zypp::source::yum;

  class PackageID {
  public:
    PackageID(string name, std::string ver, std::string rel, std::string arch)
    : _name(name)
    , _ver(ver)
    , _rel(rel)
    , _arch(arch)
    {};
    static int compare( const PackageID & lhs, const PackageID & rhs )
    {
      if (lhs._name < rhs._name)
	return 1;
      if (lhs._name > rhs._name)
	return -1;
      if (lhs._ver < rhs._ver)
	return 1;
      if (lhs._ver > rhs._ver)
	return -1;
      if (lhs._rel < rhs._rel)
	return 1;
      if (lhs._rel > rhs._rel)
	return -1;
      if (lhs._arch < rhs._arch)
	return 1;
      if (lhs._arch > rhs._arch)
	return -1;
      return 0;
    }
    string name() { return _name; }
    string ver() { return _ver; }
    string rel() { return _rel; }
    string arch() { return _arch; }
  private:
    string _name;
    string _ver;
    string _rel;
    string _arch;
  };
  inline bool operator<( const PackageID & lhs, const PackageID & rhs )
  { return PackageID::compare( lhs, rhs ) == -1; }


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

// read repomd first

  std::list<YUMRepomdData_Ptr> repo_primary;
  std::list<YUMRepomdData_Ptr> repo_files;
  std::list<YUMRepomdData_Ptr> repo_other;
  std::list<YUMRepomdData_Ptr> repo_group;
  std::list<YUMRepomdData_Ptr> repo_product;
  std::list<YUMRepomdData_Ptr> repo_patches;

  ifstream repo_st("yum/YUMParser.test/001-repomd-correct.test.xml");
  YUMRepomdParser repomd(repo_st, "");
  for(;
      ! repomd.atEnd();
      ++repomd)
  {
    if ((*repomd)->type == "primary")
      repo_primary.push_back(*repomd);
    else if ((*repomd)->type == "filelists")
      repo_files.push_back(*repomd);
    else if ((*repomd)->type == "other")
      repo_other.push_back(*repomd);
    else if ((*repomd)->type == "group")
      repo_group.push_back(*repomd);
    else if ((*repomd)->type == "product")
      repo_product.push_back(*repomd);
    else if ((*repomd)->type == "patches")
      repo_patches.push_back(*repomd);
    else
      ERR << "Unknown type of repo file: " << (*repomd)->type << endl;
  }

  // now put other and filelist data to structures for easier find
  map<PackageID, YUMFileListData_Ptr> files_data;
  map<PackageID, YUMOtherData_Ptr> other_data;
  for (std::list<YUMRepomdData_Ptr>::const_iterator it = repo_files.begin();
       it != repo_files.end();
       it++)
  {
    // TODO check checksum
    string filename = (*it)->location;
    DBG << "Reading file " << filename << endl;
    ifstream st(filename.c_str());
    YUMFileListParser filelist(st, "");
    for (;
	 ! filelist.atEnd();
	 ++filelist)
    {
      PackageID id((*filelist)->name,
		   (*filelist)->ver,
		   (*filelist)->rel,
		   (*filelist)->arch);
      files_data[id] = *filelist;
    }
    if (filelist.errorStatus())
      throw *filelist.errorStatus();
  }

  for (std::list<YUMRepomdData_Ptr>::const_iterator it = repo_other.begin();
       it != repo_other.end();
       it++)
  {
    // TODO check checksum
    string filename = (*it)->location;
    DBG << "Reading file " << filename << endl;
    ifstream st(filename.c_str());
    YUMOtherParser other(st, "");
    for (;
	 ! other.atEnd();
	 ++other)
    {
      PackageID id((*other)->name,
		   (*other)->ver,
		   (*other)->rel,
		   (*other)->arch);
      other_data[id] = *other;
    }
    if (other.errorStatus())
      throw *other.errorStatus();
  }

  // now read primary data
  for (std::list<YUMRepomdData_Ptr>::const_iterator it = repo_primary.begin();
       it != repo_primary.end();
       it++)
  {
    // TODO check checksum
    string filename = (*it)->location;
    DBG << "Reading file " << filename << endl;
    ifstream st(filename.c_str());
    YUMPrimaryParser prim(st, "");
    for (;
	 !prim.atEnd();
	 ++prim)
    {
      PackageID id((*prim)->name,
		   (*prim)->ver,
		   (*prim)->rel,
		   (*prim)->arch);
      map<PackageID, YUMOtherData_Ptr>::iterator found_other
	  = other_data.find(id);
      map<PackageID, YUMFileListData_Ptr>::iterator found_files
	  = files_data.find(id);
  
      YUMFileListData filelist_empty;
      YUMOtherData other_empty;
      Package::Ptr p = src.createPackage(
	**prim,
	found_files != files_data.end()
	  ? *found_files->second
	  : filelist_empty,
	found_other != other_data.end()
	  ? *found_other->second
	  : other_empty
      );
    }
    if (prim.errorStatus())
      throw *prim.errorStatus();
  }

  // and now other metadata
  // groups
  for (std::list<YUMRepomdData_Ptr>::const_iterator it = repo_group.begin();
       it != repo_group.end();
       it++)
  {
    // TODO check checksum
    string filename = (*it)->location;
    DBG << "Reading file " << filename << endl;
    ifstream st(filename.c_str());
    YUMGroupParser group(st, "");
    for (;
	 !group.atEnd();
	 ++group)
    {
      Selection::Ptr p = src.createGroup(**group);
    }
    if (group.errorStatus())
      throw *group.errorStatus();
  }

  // products
  for (std::list<YUMRepomdData_Ptr>::const_iterator it = repo_product.begin();
       it != repo_product.end();
       it++)
  {
    // TODO check checksum
    string filename = (*it)->location;
    DBG << "Reading file " << filename << endl;
    ifstream st(filename.c_str());
    YUMProductParser product(st, "");
    for (;
	 !product.atEnd();
	 ++product)
    {
      Product::Ptr p = src.createProduct(**product);
    }
    if (product.errorStatus())
      throw *product.errorStatus();
  }

  // patches
  std::list<std::string> patch_files;
  for (std::list<YUMRepomdData_Ptr>::const_iterator it = repo_patches.begin();
       it != repo_patches.end();
       it++)
  {
    // TODO check checksum
    string filename = (*it)->location;
    DBG << "Reading file " << filename << endl;
    ifstream st(filename.c_str());
    YUMPatchesParser patch(st, "");
    for (;
	 !patch.atEnd();
	 ++patch)
    {
      // TODO check checksum
      string filename = (*patch)->location;
      patch_files.push_back(filename);
    }
    if (patch.errorStatus())
      throw *patch.errorStatus();
  }

try {

  for (std::list<std::string>::const_iterator it = patch_files.begin();
       it != patch_files.end();
       it++)
  {
      std::string filename = *it;
      DBG << "Reading file " << filename << endl;
      ifstream st(filename.c_str());
      YUMPatchParser ptch(st, "");
      for(;
          !ptch.atEnd();
          ++ptch)
      {
ERR << "Creating patch" << endl;
	Patch::Ptr p = src.createPatch(**ptch);
      }
      if (ptch.errorStatus())
	throw *ptch.errorStatus();
  }

}
catch (const Exception & excpt_r)
{
  ERR << excpt_r << endl;
}
catch (...)
{
  ERR << "exception" << endl;
}
// read data then

  INT << "===[END]============================================" << endl;
  return 0;
}
