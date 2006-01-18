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

#include "zypp/source/yum/YUMSourceImpl.h"

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
using namespace zypp::parser::yum;
using namespace zypp::source::yum;

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

std::string
XMLFilesBackend::dirForResolvable( Resolvable::Ptr resolvable ) const
{
  std::string dir;
  // FIXME replace with path class of boost
  dir += std::string(ZYPP_DB_DIR);
  dir += "/";
  dir += typeToString(resolvable, true);
  return dir;
}

std::string
XMLFilesBackend::fullPathForResolvable( Resolvable::Ptr resolvable ) const
{
  std::string filename;
  filename = dirForResolvable(resolvable) + "/";
  filename += resolvable->name();
  return filename;
}

void
XMLFilesBackend::storeObject( Resolvable::Ptr resolvable )
{
  std::string xml = castedToXML(resolvable);
  std::string filename = fullPathForResolvable(resolvable);
  DBG << std::endl << xml << std::endl;
  std::ofstream file;
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
  zypp::source::yum::YUMSourceImpl src;
  std::list<Resolvable::Kind> kinds;
  // only patches for now
  kinds.push_back(ResTraits<zypp::Patch>::kind);

  std::list<Resolvable::Kind>::const_iterator it_kinds;
  for ( it_kinds = kinds.begin() ; it_kinds != kinds.end(); ++it_kinds )
  {
    // patches
    if ( (*it_kinds) == ResTraits<zypp::Patch>::kind )
    {
      DBG << "parches...";
    }
  }
  //for YUMPatchParser parser;
/*
  return  std::list<Resolvable::Ptr>();
  for ( fs::directory_iterator dir_itr( dirForResolvable );
          dir_itr != end_iter;
          ++dir_itr )
    {
      try
      {
        if ( fs::is_directory( *dir_itr ) )
        {
          ++dir_count;
          std::cout << dir_itr->leaf()<< " [directory]\n";
        }
        else
        {
          ++file_count;
          std::cout << dir_itr->leaf() << "\n";
        }
      }
      catch ( const std::exception & ex )
      {
        ++err_count;
        std::cout << dir_itr->leaf() << " " << ex.what() << std::endl;
      }
    }
*/
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
