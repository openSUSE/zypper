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
#include <cstdlib>

#include "zypp/base/Logger.h"

#include "zypp/source/yum/YUMSourceImpl.h"
#include "zypp/parser/yum/YUMParser.h"

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
  bool randomFileName;
	YUMSourceImpl source;
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
  d = new Private;
  d->randomFileName = false;
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

// Taken from KApplication
int XMLFilesBackend::random() const
{
   static bool init = false;
   if (!init)
   {
      unsigned int seed;
      init = true;
      int fd = open("/dev/urandom", O_RDONLY);
      if (fd < 0 || ::read(fd, &seed, sizeof(seed)) != sizeof(seed))
      {
            // No /dev/urandom... try something else.
            srand(getpid());
            seed = rand()+time(0);
      }
      if (fd >= 0) close(fd);
      srand(seed);
   }
   return rand();
}

// Taken from KApplication
std::string XMLFilesBackend::randomString(int length) const
{
   if (length <=0 ) return std::string();

   std::string str; str.resize( length );
   int i = 0;
   while (length--)
   {
      int r=random() % 62;
      r+=48;
      if (r>57) r+=7;
      if (r>90) r+=6;
      str[i++] =  char(r);
      // so what if I work backwards?
   }
   return str;
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

void XMLFilesBackend::setRandomFileNameEnabled( bool enabled )
{
  d->randomFileName = enabled;
}

std::string
XMLFilesBackend::dirForResolvableKind( Resolvable::Kind kind ) const
{
  std::string dir;
  // FIXME replace with path class of boost
  dir += std::string(ZYPP_DB_DIR);
  dir += "/";
  dir += resolvableKindToString(kind, true);
  return dir;
}

std::string
XMLFilesBackend::dirForResolvable( Resolvable::constPtr resolvable ) const
{
  return dirForResolvableKind(resolvable->kind());
}

std::string
XMLFilesBackend::fullPathForResolvable( Resolvable::constPtr resolvable ) const
{
  std::string filename;
  filename = dirForResolvable(resolvable) + "/";
  filename += d->randomFileName ? randomString(40) : resolvable->name();
  return filename;
}

void
XMLFilesBackend::storeObject( Resolvable::constPtr resolvable )
{
  std::string xml = castedToXML(resolvable);
  std::string filename = fullPathForResolvable(resolvable);
  //DBG << std::endl << xml << std::endl;
  std::ofstream file;
  //DBG << filename << std::endl;
  file.open(filename.c_str());
  file << xml;
  file.close();
}

void
XMLFilesBackend::deleteObject( Resolvable::Ptr resolvable )
{}

Resolvable::Ptr XMLFilesBackend::resolvableFromFile( std::string file_path, Resolvable::Kind kind ) const
{
  //DBG << "[" << resolvableKindToString( kind, false ) << "] - " << file_path << std::endl;
  Resolvable::Ptr resolvable;
  std::ifstream res_file(file_path.c_str());
  if ( kind == ResTraits<zypp::Patch>::kind )
  {
    // a patch file can contain more than one patch, but we store only
    // one patch, so we break at the first
    // FIXME how can we avoid creating this for every object?
    YUMPatchParser iter(res_file,"");
    for (; !iter.atEnd(); ++iter)
    {
      resolvable = d->source.createPatch(**iter);
      break;
    }
  }
  else
  {
    resolvable = 0;
  }
  return resolvable;
}

std::list<Resolvable::Ptr>
XMLFilesBackend::storedObjects()
{
  std::list<Resolvable::Ptr> objects;
  std::list<Resolvable::Kind> kinds;
  // only patches for now
  kinds.push_back(ResTraits<zypp::Patch>::kind);

  std::list<Resolvable::Kind>::const_iterator it_kinds;
  for ( it_kinds = kinds.begin() ; it_kinds != kinds.end(); ++it_kinds )
  {
    Resolvable::Kind kind = (*it_kinds);
    // patches
    if ( kind == ResTraits<zypp::Patch>::kind )
    {
      std::string dir_path = dirForResolvableKind(kind);
      DBG << "objects in ... " << dir_path << std::endl;
      directory_iterator end_iter;
      if ( !exists( dir_path ) ) continue;
      for ( directory_iterator dir_itr( dir_path ); dir_itr != end_iter; ++dir_itr )
      {
        //DBG << "[" << resolvableKindToString( kind, false ) << "] - " << dir_itr->leaf() << std::endl;
        objects.push_back( resolvableFromFile( dir_path + "/" + dir_itr->leaf(), kind) );
      }
    }
  }
  return objects;
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

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : XMLFilesBackend::~XMLFilesBackend
//	METHOD TYPE : Dtor
//
XMLFilesBackend::~XMLFilesBackend()
{
  delete d;
}

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
