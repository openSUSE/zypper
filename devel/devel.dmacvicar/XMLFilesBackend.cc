/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	devel/devel.dmacvicar/XMLFilesBackend.cc
*
*/
#include <iostream>
#include <ctime>
#include "zypp/base/Logger.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <streambuf>

#include "boost/filesystem/operations.hpp" // includes boost/filesystem/path.hpp
#include "boost/filesystem/fstream.hpp"    // ditto

#include "XMLFilesBackend.h"
#include "serialize.h"
#include <list>

#define ZYPP_DB_DIR "zypp_db"

using std::endl;
using namespace boost::filesystem;
//using namespace boost::iostreams;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
namespace storage
{ /////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : XMLFilesBackend::Private
//
///////////////////////////////////////////////////////////////////
class XMLFilesBackend::Private
{
	public:
};

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : XMLFilesBackend
//
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : XMLFilesBackend::XMLFilesBackend
//	METHOD TYPE : Ctor
//
XMLFilesBackend::XMLFilesBackend()
{
	// check if the db exists
	if (!isDatabaseInitialized())
	{
		DBG << "Database not initialized" << std::endl;
		initDatabaseForFirstTime();
		if (!isDatabaseInitialized())
			DBG << "Error, cant init database" << std::endl;
		else
			DBG << "Database initialized" << std::endl;
	}
	else
	{
		DBG << "Database already initialized" << std::endl;
	}
}

bool
XMLFilesBackend::isDatabaseInitialized()
{
  return exists( ZYPP_DB_DIR );
}

void
XMLFilesBackend::initDatabaseForFirstTime()
{
  // FIXME duncan * handle exceptions
  DBG << "Creating directory structure..." << std::endl;
  create_directory( path(ZYPP_DB_DIR) );
  create_directory( path(ZYPP_DB_DIR) / path("patches") );
  create_directory( path(ZYPP_DB_DIR) / path("selections") );
  create_directory( path(ZYPP_DB_DIR) / path("products") );
}

std::string
XMLFilesBackend::randomFileName() const
{
  FILE *fp;
  char puffer[49];
  if ( (fp = popen("openssl rand -base64 48", "r")) == 0)
  {
    DBG << "mierda!" << std::endl;
    //ZYPP_THROW("put some message here");
  }
  fscanf(fp, "%s", &puffer);
  pclose(fp);
  return "blah";
}

std::string XMLFilesBackend::fileNameForPatch( Patch::Ptr patch ) const
{
  return patch->id();
}

std::list<Patch::Ptr>
XMLFilesBackend::installedPatches()
{
  return std::list<Patch::Ptr>();
}

void
XMLFilesBackend::storePatch( Patch::Ptr p )
{
  std::string xml = toXML(p);
  DBG << std::endl << xml << std::endl;
  std::ofstream file;
  // FIXME replace with path class of boost
  file.open( (std::string(ZYPP_DB_DIR) + std::string("/patches/") + fileNameForPatch(p)).c_str() );
  file << xml;
  file.close();
}

void
XMLFilesBackend::insertTest()
{

}

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : XMLFilesBackend::~XMLFilesBackend
//	METHOD TYPE : Dtor
//
XMLFilesBackend::~XMLFilesBackend()
{}

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : XMLFilesBackend::doTest()
//	METHOD TYPE : Dtor
//
void XMLFilesBackend::doTest()
{}

/******************************************************************
**
**	FUNCTION NAME : operator<<
**	FUNCTION TYPE : std::ostream &
*/
std::ostream & operator<<( std::ostream & str, const XMLFilesBackend & obj )
{
	return str;
}

		/////////////////////////////////////////////////////////////////
	} // namespace devel.dmacvicar
	///////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////
} // namespace devel
///////////////////////////////////////////////////////////////////
