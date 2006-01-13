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
	if (!isBackendInitialized())
	{
		DBG << "Database not initialized" << std::endl;
		initBackend();
    // should be initialized now...
		if (!isBackendInitialized())
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
XMLFilesBackend::isBackendInitialized()
{
  return exists( ZYPP_DB_DIR );
}

void
XMLFilesBackend::initBackend()
{
  // FIXME duncan * handle exceptions
  DBG << "Creating directory structure..." << std::endl;
  create_directory( path(ZYPP_DB_DIR) );
  create_directory( path(ZYPP_DB_DIR) / path("patches") );
  create_directory( path(ZYPP_DB_DIR) / path("selections") );
  create_directory( path(ZYPP_DB_DIR) / path("products") );
}

void
XMLFilesBackend::storeObject( Resolvable::Ptr resolvable )
{
  std::string xml = castedToXML(resolvable);
  std::string filename;
  DBG << std::endl << xml << std::endl;
  std::ofstream file;
  // FIXME replace with path class of boost
  filename += std::string(ZYPP_DB_DIR);
  filename += "/";
  filename += typeToString(resolvable, true);
  filename += "/";
  filename += resolvable->name();
  DBG << filename << std::endl;
  file.open(filename.c_str());
  file << xml;
  file.close();
}

void
XMLFilesBackend::deleteObject( Resolvable::Ptr resolvable )
{}

std::list<Resolvable::Ptr>
XMLFilesBackend::storedObjects()
{
  return  std::list<Resolvable::Ptr>();
}

std::list<Resolvable::Ptr>
XMLFilesBackend::storedObjects(const Resolvable::Kind)
{
  return storedObjects();
}

std::list<Resolvable::Ptr>
XMLFilesBackend::storedObjects(const Resolvable::Kind, const std::string & name, bool partial_match)
{
  return storedObjects();
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
