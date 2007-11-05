/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/target/store/XMLFilesBackend.cc
*
*/
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <cstdio>

#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/base/Random.h"
#include "zypp/base/Gettext.h"
#include "zypp/CapFactory.h"
#include "zypp/Digest.h"
#include "zypp/ExternalProgram.h"

#include "zypp/target/store/xml/XMLPatchImpl.h"
#include "zypp/target/store/xml/XMLMessageImpl.h"
#include "zypp/target/store/xml/XMLScriptImpl.h"
#include "zypp/target/store/xml/XMLSelectionImpl.h"
#include "zypp/target/store/xml/XMLProductImpl.h"
#include "zypp/target/store/xml/XMLPatternImpl.h"
#include "zypp/target/store/xml/XMLAtomImpl.h"

#include "zypp/parser/xmlstore/XMLProductParser.h"
#include "zypp/parser/xmlstore/XMLPatternParser.h"
#include "zypp/parser/xmlstore/XMLPatchParser.h"
#include "zypp/parser/xmlstore/XMLLanguageParser.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <streambuf>

#include <list>

#include <zypp/TmpPath.h>
#include <zypp/ZYppFactory.h>
#include <zypp/ZYpp.h>
#include <zypp/PathInfo.h>

#include "XMLFilesBackend.h"
#include "serialize.h"

//#define ZYPP_DB_DIR "/var/lib/zypp_db/"
#define ZYPP_DB_DIR ( getZYpp()->homePath().asString()+"/db/" )

using std::endl;
using std::string;
using std::list;
using namespace zypp;
using namespace zypp::filesystem;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
namespace storage
{ /////////////////////////////////////////////////////////////////

/**
 * the following hardcoded table fixes a bug where Product was
 *  serialized to the store using distproduct and distversion
 *  in the name/version fields.
 *  Those products have missing distversion.
 *  The trick is to see the version of the xml format, if it is
 *  minor than 2.0, we check if the name matches a dist
 *  product and fix the product on construction
 *
 *  see: https://bugzilla.novell.com/show_bug.cgi?id=205392
 */

struct PRODUCT_TABLE_ENTRY
{
  const char * dist_name;
  const char * dist_version;
  const char * product_name;
  const char * product_version;
};

/**
 * create the map on demand so we
 * create it once and only when
 * needed
 */
PRODUCT_TABLE_ENTRY* products_table()
{
  static PRODUCT_TABLE_ENTRY products[] = {
    { "SUSE-Linux-Enterprise-Desktop-i386", "10-0", "SUSE SLED" , "10" },
    { "SUSE-Linux-Enterprise-Desktop-x86_64", "10-0", "SUSE SLED", "10" },
    { "SUSE-Linux-Enterprise-Server-i386", "10-0", "SUSE SLES", "10" },
    { "SUSE-Linux-Enterprise-Server-x86_64", "10-0", "SUSE SLES", "10" },
    { "SUSE-Linux-Enterprise-Server-ppc", "10-0", "SUSE SLES", "10" },
    { "SUSE-Linux-Enterprise-Server-ia64", "10-0", "SUSE SLES", "10" },
    { "SUSE-Linux-Enterprise-Server-s390x", "10-0", "SUSE SLES", "10" },
    { "SUSE-Linux-10.1-CD-download-x86", "10.1-0", "SUSE LINUX", "10.1" },
    { "SUSE-Linux-10.1-CD-download-ppc", "10.1-0", "SUSE LINUX", "10.1" },
    { "SUSE-Linux-10.1-CD-download-x86_64", "10.1-0", "SUSE LINUX", "10.1" },
    { "SUSE-Linux-10.1-CD-x86", "10.1-0", "SUSE LINUX", "10.1" },
    { "SUSE-Linux-10.1-CD-ppc", "10.1-0", "SUSE LINUX", "10.1" },
    { "SUSE-Linux-10.1-CD-x86_64", "10.1-0", "SUSE LINUX", "10.1" },
    { "SUSE-Linux-10.1-DVD9-x86-x86_64", "10.1-0", "SUSE LINUX", "10.1" },
    { "SUSE-Linux-10.1-OSS-DVD-x86", "10.1-0", "SUSE LINUX", "10.1" },
    { "SUSE-Linux-10.1-DVD-OSS-i386", "10.1-0", "SUSE LINUX", "10.1" },
    { "SUSE-Linux-10.1-DVD-OSS-ppc", "10.1-0", "SUSE LINUX", "10.1" },
    { "SUSE-Linux-10.1-DVD-OSS-x86_64", "10.1-0", "SUSE LINUX", "10.1" },
    { "SUSE-Linux-10.1-FTP", "10.1-0", "SUSE LINUX", "10.1" },
    { "SUSE-Linux-10.1-DVD-i386", "10.1-0", "SUSE LINUX", "10.1" },
    { "SUSE-Linux-10.1-DVD-x86_64", "10.1-0", "SUSE LINUX", "10.1" },
    { "SuSE-Linux-10.1-PromoDVD-i386", "10.1-0", "SUSE LINUX", "10.1" },
    { "SUSE-Linux-10.1-DVD9-CTMAGAZIN-x86-x86_64", "10.1-0", "SUSE LINUX", "10.1" },
    { "SUSE-Linux-Enterprise-SDK-i386", "10-0", "SLE SDK", "10" },
    { "SUSE-Linux-Enterprise-SDK-x86_64", "10-0", "SLE SDK", "10" },
    { "SUSE-Linux-Enterprise-SDK-ia64", "10-0", "SLE SDK", "10" },
    { "SUSE-Linux-Enterprise-SDK-s390x", "10-0", "SLE SDK", "10" },
    { "SUSE-Linux-Enterprise-SDK-ppc", "10-0", "SLE SDK", "10" },
    { "SUSE-Linux-Enterprise-RT", "10-0", "SLE RT", "10" },
    { 0L, 0L, 0L, 0L }
  };

  return products;
}

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : XMLFilesBackend::Private
//
///////////////////////////////////////////////////////////////////
class XMLFilesBackend::Private
{
  public:
  Private()
  { }
  bool randomFileName;
  std::set<Resolvable::Kind> kinds;
  std::set<Resolvable::Kind> kinds_flags;
  Pathname root;
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
XMLFilesBackend::XMLFilesBackend(const Pathname &root) : Backend(root)
{
  d = new Private;
  d->randomFileName = false;
  d->root = root;

  // types of resolvables stored (supported)
  d->kinds.insert(ResTraits<zypp::Patch>::kind);
  //d->kinds.insert(ResTraits<zypp::Message>::kind);
  //d->kinds.insert(ResTraits<zypp::Script>::kind);
  d->kinds.insert(ResTraits<zypp::Selection>::kind);
  d->kinds.insert(ResTraits<zypp::Product>::kind);
  d->kinds.insert(ResTraits<zypp::Pattern>::kind);
  d->kinds.insert(ResTraits<zypp::Language>::kind);

  // types of resolvables stored (supported)
  d->kinds_flags.insert(ResTraits<zypp::Package>::kind);
  d->kinds_flags.insert(ResTraits<zypp::Patch>::kind);
  //d->kinds.insert(ResTraits<zypp::Message>::kind);
  d->kinds_flags.insert(ResTraits<zypp::Script>::kind);
  d->kinds_flags.insert(ResTraits<zypp::Selection>::kind);
  d->kinds_flags.insert(ResTraits<zypp::Product>::kind);
  d->kinds_flags.insert(ResTraits<zypp::Pattern>::kind);
  d->kinds_flags.insert(ResTraits<zypp::Language>::kind);


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

Date XMLFilesBackend::timestamp() const
{
  PathInfo ts_info = PathInfo( d->root + Pathname(ZYPP_DB_DIR) + "timestamp" );
  if ( ts_info.isExist() )
  {
    return Date(ts_info.mtime());
  }
  else
  {
    updateTimestamp();
    return Date::now();
  }
}

bool
XMLFilesBackend::isBackendInitialized() const
{
  bool ok = true;
  Pathname dbdir = d->root +  ZYPP_DB_DIR;
  ok = ok && PathInfo(dbdir).isExist();

  bool fixperms = false;

  // The db dir was created with 700 permissions
  // see bug #169094
  if ( ok && PathInfo(dbdir).perm() == 0700 )
  {
    if ( geteuid() == 0 )
    {
      fixperms = true;
      WAR << "Wrong permissions for /var/lib/zypp, will fix" << std::endl;

      const char* argv[] =
      {
        "chmod",
        "-R",
        "0755",
        "/var/lib/zypp",
        NULL
      };

      ExternalProgram prog(argv,ExternalProgram::Discard_Stderr, false, -1, true);
      prog.close();
    }
    else
    {
      WAR << "Wrong permissions for /var/lib/zypp, but can't fix unless you run as root." << std::endl;
    }

  }

  // folders for resolvables
  std::set<Resolvable::Kind>::const_iterator it_kinds;
  for ( it_kinds = d->kinds.begin() ; it_kinds != d->kinds.end(); ++it_kinds )
  {
    Resolvable::Kind kind = (*it_kinds);
    bool isthere = PathInfo( dirForResolvableKind(kind) ).isExist();
    ok = ok && isthere;
  }

  // resolvable flags folder flags
  for ( it_kinds = d->kinds_flags.begin() ; it_kinds != d->kinds_flags.end(); ++it_kinds )
  {
    Resolvable::Kind kind = (*it_kinds);
    bool isthere = PathInfo( dirForResolvableKindFlags(kind) ).isExist();
    ok = ok && isthere;
  }

  // named flags
  bool nmthere = PathInfo( dirForNamedFlags() ).isExist();
  ok = ok && nmthere;

  return ok;
}

void
XMLFilesBackend::initBackend()
{
  // all directories are 755 except the sources directory which can contain passwords
  // in urls.

  Pathname topdir = d->root + Pathname(ZYPP_DB_DIR);
  DBG << "Creating directory structure " << topdir << std::endl;

  if (0 != assert_dir(topdir, 0755))
      ZYPP_THROW(Exception("Cannot create XMLBackend db directory " + topdir.asString()));

  // create dir for resolvables
  std::set<Resolvable::Kind>::const_iterator it_kinds;
  for ( it_kinds = d->kinds.begin() ; it_kinds != d->kinds.end(); ++it_kinds )
  {
    Resolvable::Kind kind = (*it_kinds);
    Pathname p(dirForResolvableKind(kind));
    if (0 != assert_dir(p, 0755))
      ZYPP_THROW(Exception("Cannot create directory " + p.asString()));
    else
      MIL << "Created " << p.asString() << std::endl;
  }

  // create dir for resolvables flags
  for ( it_kinds = d->kinds_flags.begin() ; it_kinds != d->kinds_flags.end(); ++it_kinds )
  {
    Resolvable::Kind kind = (*it_kinds);
    Pathname p(dirForResolvableKindFlags(kind));
    if (0 != assert_dir(p, 0755))
      ZYPP_THROW(Exception("Cannot create directory " + p.asString()));
    else
      MIL << "Created " << p.asString() << std::endl;
  }

  // dir for named flags
  Pathname namedflags(dirForNamedFlags());
  if (0 != assert_dir(namedflags, 0755))
    ZYPP_THROW(Exception("Cannot create directory " + namedflags.asString()));
  else
    MIL << "Created " << namedflags.asString() << std::endl;
}

void XMLFilesBackend::setRandomFileNameEnabled( bool enabled )
{
  d->randomFileName = enabled;
}

std::string
XMLFilesBackend::dirForResolvableKind( Resolvable::Kind kind ) const
{
  return Pathname( d->root + Pathname(ZYPP_DB_DIR) + Pathname(resolvableKindToString(kind, true)) ).asString();
}

std::string
XMLFilesBackend::dirForResolvableKindFlags( Resolvable::Kind kind ) const
{
  return Pathname( d->root + Pathname(ZYPP_DB_DIR) + Pathname("flags") + Pathname(resolvableKindToString(kind, true)) ).asString();
}

std::string
XMLFilesBackend::dirForNamedFlags() const
{
  return Pathname( d->root + Pathname(ZYPP_DB_DIR) + Pathname("named-flags")).asString();
}

std::string
XMLFilesBackend::dirForResolvable( ResObject::constPtr resolvable ) const
{
  return dirForResolvableKind(resolvable->kind());
}

std::string
XMLFilesBackend::dirForResolvableFlags( ResObject::constPtr resolvable ) const
{
  return dirForResolvableKindFlags(resolvable->kind());
}

std::string
XMLFilesBackend::fileNameForNVR( const NVR &nvr ) const
{
  std::string filename;
  filename = nvr.name;
  if ( nvr.edition != Edition::noedition )
  {
     filename += "-" + nvr.edition.asString();
  }
  // get rid of spaces and other characters
  std::stringstream filename_stream(filename);
  std::string filename_encoded = Digest::digest("MD5", filename_stream);
  return filename_encoded;
}

std::string
XMLFilesBackend::fileNameForResolvable( ResObject::constPtr resolvable ) const
{
  return fileNameForNVR( NVR( resolvable->name(), resolvable->edition() ) );
}

std::string
XMLFilesBackend::fullPathForResolvable( ResObject::constPtr resolvable ) const
{
  return( Pathname( dirForResolvable(resolvable) ) / fileNameForResolvable(resolvable) ).asString();
}

std::string
XMLFilesBackend::fullPathForNamedFlags( const std::string &key ) const
{
  std::stringstream key_stream(key);
  std::string key_encoded = Digest::digest("MD5", key_stream);
  return( Pathname( dirForNamedFlags() ) / key_encoded ).asString();
}

std::string
XMLFilesBackend::fullPathForResolvableFlags( ResObject::constPtr resolvable ) const
{
  // flags are in a hidden file with the same name
  return( Pathname( dirForResolvableFlags(resolvable) ) / fileNameForResolvable(resolvable) ).asString();
}

void
XMLFilesBackend::setObjectFlag( ResObject::constPtr resolvable, const std::string &flag )
{
  std::set<std::string> flags = objectFlags(resolvable);
  flags.insert(flag);
  writeObjectFlags(resolvable, flags);
}

void
XMLFilesBackend::removeObjectFlag( ResObject::constPtr resolvable, const std::string &flag )
{
  std::set<std::string> flags = objectFlags(resolvable);
  flags.erase(flag);
  writeObjectFlags(resolvable, flags);
}

void
XMLFilesBackend::writeObjectFlags( ResObject::constPtr resolvable, const std::set<std::string> &flags )
{
  std::string filename = fullPathForResolvableFlags(resolvable);
  writeFlagsInFile( filename, flags );
  MIL << "Wrote " << flags.size() << " flags for " << resolvable->name() << " " << resolvable->edition() << std::endl;
}

std::set<std::string>
XMLFilesBackend::objectFlags( ResObject::constPtr resolvable ) const
{
  std::string filename = fullPathForResolvableFlags(resolvable);
  return flagsFromFile(filename);
}

bool
XMLFilesBackend::doesObjectHasFlag( ResObject::constPtr resolvable, const std::string &flag ) const
{
  std::set<std::string> flags = objectFlags(resolvable);
  return (find(flags.begin(), flags.end(), flag) != flags.end());
}

/////////////////////////////////////////////////////////
// Named Flags API
////////////////////////////////////////////////////////

void
XMLFilesBackend::setFlag( const std::string &key, const std::string &flag )
{
  std::set<std::string> _flags = flags(key);
  _flags.insert(flag);
  writeFlags(key, _flags);
}

void
XMLFilesBackend::removeFlag( const std::string &key, const std::string &flag )
{
  std::set<std::string> _flags = flags(key);
  _flags.erase(flag);
  writeFlags(key, _flags);
}

std::set<std::string>
XMLFilesBackend::flags( const std::string &key ) const
{
  std::string filename = fullPathForNamedFlags(key);
  return flagsFromFile(filename);
}

bool
XMLFilesBackend::hasFlag( const std::string &key, const std::string &flag ) const
{
  std::set<std::string> _flags = flags(key);
  return (find(_flags.begin(), _flags.end(), flag) != _flags.end());
}

void
XMLFilesBackend::writeFlags( const std::string &key, const std::set<std::string> &pflags )
{
  std::string filename = fullPathForNamedFlags(key);
  writeFlagsInFile( filename, pflags );
  MIL << "Wrote " << pflags.size() << " flags for " << key << std::endl;
}

/////////////////////////////////////////////////////////
// Common functions for both named and resolvable flags
////////////////////////////////////////////////////////

void
XMLFilesBackend::writeFlagsInFile( const std::string &filename, const std::set<std::string> &pflags )
{
  std::ofstream file(filename.c_str());
  if (!file) {
    ZYPP_THROW (Exception( "Can't open " + filename ) );
  }

  try
  {
    for ( std::set<std::string>::const_iterator it = pflags.begin(); it != pflags.end(); it++)
    {
      // dont save empty strings
      if ( *it == std::string() )
        continue;
      else
        file << *it << std::endl;
    }
    file << std::endl;
    MIL << "Wrote " << pflags.size() << " flags in " << filename << std::endl;
  }
  catch( std::exception &e )
  {
    ZYPP_THROW(Exception("Can't write flags to store"));
  }
  updateTimestamp();
}

std::set<std::string>
XMLFilesBackend::flagsFromFile( const std::string &filename ) const
{
  std::set<std::string> _flags;
  // do we have previous saved flags?
  if ( ! PathInfo( filename ).isExist() )
    return _flags;

  std::ifstream file(filename.c_str());
  if (!file) {
    ZYPP_THROW (Exception( "Can't open " + filename ) );
  }

  std::string buffer;
  while(file && !file.eof())
  {
    getline(file, buffer);
    if (buffer == std::string())
      continue;

    _flags.insert(buffer);
  }
  //MIL << "Read " << flags.size() << " flags for " << resolvable->name() << " " << resolvable->edition() << std::endl;
  return _flags;
}

void
XMLFilesBackend::updateTimestamp() const
{
  Pathname filename = d->root + Pathname(ZYPP_DB_DIR) + "timestamp";
  std::ofstream file(filename.asString().c_str(), std::ios::out);
  if (!file)
  {
    ZYPP_THROW (Exception( "Can't open timestamp file " + filename.asString() ) );
  }
  file.close();
}

/////////////////////////////////////////////////////////
// Resolvables storage
////////////////////////////////////////////////////////

void
XMLFilesBackend::storeObject( ResObject::constPtr resolvable )
{
  // only ignore if it is not a supported resolvable kind
  std::set<Resolvable::Kind>::const_iterator it;
  it = find(d->kinds.begin(), d->kinds.end(), resolvable->kind() );
  if (it == d->kinds.end())
  {
    ERR << "This backend was not designed to store resolvable of kind " << resolvableKindToString(resolvable->kind()) << ", ignoring..." << std::endl;
    return;
  }

  std::string xml = castedToXML(resolvable);
  std::string filename = fullPathForResolvable(resolvable);
  //DBG << std::endl << xml << std::endl;
  std::ofstream file;
  //DBG << filename << std::endl;
  try
  {
    file.open(filename.c_str());
    file << xml;
    file.close();
  }
  catch(std::exception &e)
  {
    ERR << "Error saving resolvable " << resolvable << std::endl;
    ZYPP_THROW(Exception(e.what()));
  }
  updateTimestamp();
}

void
XMLFilesBackend::deleteFileObject( const Pathname &filename ) const
{
  try
  {
    int ret = filesystem::unlink( filename );
    if ( ret != 0 && ret != ENOENT )
    {
      ERR << "Error removing resolvable file " << filename << std::endl;
      ZYPP_THROW(Exception("Error deleting " + filename.asString()));
    }
    updateTimestamp();
  }
  catch(std::exception &e)
  {
    ERR << "Error removing resolvable file " << filename << std::endl;
    ZYPP_THROW(Exception(e.what()));
  }
}

void
XMLFilesBackend::deleteObject( ResObject::constPtr resolvable )
{
  // only ignore if it is not a supported resolvable kind
  std::set<Resolvable::Kind>::const_iterator it;
  it = find(d->kinds.begin(), d->kinds.end(), resolvable->kind() );
  if (it == d->kinds.end())
  {
    ERR << "This backend was not designed to store resolvable of kind " << resolvableKindToString(resolvable->kind()) << ", ignoring..." << std::endl;
    return;
  }

  // only remove the file
  try
  {
    deleteFileObject( fullPathForResolvable(resolvable) );
  }
  catch ( const Exception &e )
  {
    ERR << "Error removing resolvable " << resolvable << std::endl;
    ZYPP_RETHROW(e);
  }
}

std::list<ResObject::Ptr> XMLFilesBackend::resolvablesFromFile( std::string file_path, Resolvable::Kind kind ) const
{
  MIL << "[" << resolvableKindToString( kind, false ) << "] - " << file_path << std::endl;
  std::list<ResObject::Ptr> resolvables;
  std::ifstream res_file(file_path.c_str());
  if ( kind == ResTraits<zypp::Patch>::kind )
  {
    // a patch file can contain more than one patch, but we store only
    // one patch, so we break at the first
    // FIXME how can we avoid creating this for every object?
    try {
    XMLPatchParser iter(res_file,"");
    for (; !iter.atEnd(); ++iter)
    {
      Patch::Ptr patch = createPatch(**iter);
      resolvables.push_back(patch);
      Patch::AtomList atoms = patch->atoms();
      for (Patch::AtomList::iterator at = atoms.begin(); at != atoms.end(); at++)
        resolvables.push_back(*at);

      break;
    }
    }
    catch (const Exception & excpt_r)
    {
	ZYPP_CAUGHT( excpt_r );
	WAR << "Skipping invalid patch file " << file_path << endl;
    }
  }
  else if ( kind == ResTraits<zypp::Product>::kind )
  {
    XMLProductParser iter(res_file,"");
    for (; !iter.atEnd(); ++iter)
    {
      resolvables.push_back(createProduct(**iter));
      break;
    }
  }
  else if ( kind == ResTraits<zypp::Selection>::kind )
  {
    XMLPatternParser iter(res_file,"");
    for (; !iter.atEnd(); ++iter)
    {
      resolvables.push_back(createSelection(**iter));
      break;
    }
  }
  else if ( kind == ResTraits<zypp::Pattern>::kind )
  {
    XMLPatternParser iter(res_file,"");
    for (; !iter.atEnd(); ++iter)
    {
      resolvables.push_back(createPattern(**iter));
      break;
    }
  }
  else if ( kind == ResTraits<zypp::Language>::kind )
  {
    XMLLanguageParser iter(res_file,"");
    for (; !iter.atEnd(); ++iter)
    {
      resolvables.push_back(createLanguage(**iter));
      break;
    }
  }
  else
  {
    /* nothing for now */
  }
  return resolvables;
}

std::list<ResObject::Ptr>
XMLFilesBackend::storedObjects() const
{
  DBG << std::endl;
  std::list<ResObject::Ptr> objects;

  std::set<Resolvable::Kind>::const_iterator it_kinds;
  for ( it_kinds = d->kinds.begin() ; it_kinds != d->kinds.end(); ++it_kinds )
  {
    Resolvable::Kind kind = (*it_kinds);
    std::list<ResObject::Ptr> objects_for_kind = storedObjects(kind);
    std::list<ResObject::Ptr>::iterator it;
    for( it = objects_for_kind.begin(); it != objects_for_kind.end(); ++it)
    {
      //DBG << "adding objects back" << std::endl;
      objects.push_back(*it);
    }
  }
  return objects;
}

std::list<ResObject::Ptr>
XMLFilesBackend::storedObjects(const Resolvable::Kind kind) const
{
  std::list<ResObject::Ptr> objects;
  Pathname dir_path(dirForResolvableKind(kind));
  DBG << "Reading objects of kind " << resolvableKindToString(kind) << " in " << dir_path << std::endl;
  // return empty list if the dir does not exist
  if ( ! PathInfo( dir_path ).isExist() )
  {
    ERR << "path " << dir_path << " does not exists. Required to read objects of kind " << resolvableKindToString(kind) << std::endl;
    return std::list<ResObject::Ptr>();
  }

  list<string> files;
  filesystem::readdir( files, dir_path, false /* ignore hidden .name files */ );

  for ( list<string>::const_iterator it = files.begin(); it != files.end(); ++it )
  {
    Pathname curr_file = dir_path + (*it);
    DBG << "[" << resolvableKindToString( kind, false ) << "] - " << curr_file << std::endl;
    std::list<ResObject::Ptr> objects_for_file;
    objects_for_file = resolvablesFromFile( curr_file.asString(), kind);
    for ( std::list<ResObject::Ptr>::iterator it = objects_for_file.begin(); it != objects_for_file.end(); ++it)
      objects.push_back(*it);
  }

  MIL << "done reading " <<  objects.size() << " stored objects for file of kind " << resolvableKindToString(kind) << std::endl;
  return objects;
}

std::list<ResObject::Ptr>
XMLFilesBackend::storedObjects(const Resolvable::Kind kind, const std::string & name, bool partial_match) const
{
  std::list<ResObject::Ptr> result;
  std::list<ResObject::Ptr> all;
  all = storedObjects(kind);
  std::list<ResObject::Ptr>::const_iterator it;
  for( it = all.begin(); it != all.end(); ++it)
  {
    ResObject::Ptr item = *it;
    if (item->name() == name )
      result.push_back(item);
  }
  MIL << "done reading stored objects of kind " << resolvableKindToString(kind) << " and keyword [" << name <<"]" << std::endl;
  return result;
}

Patch::Ptr
XMLFilesBackend::createPatch( const zypp::parser::xmlstore::XMLPatchData & parsed ) const
{
  try
  {
    detail::ResImplTraits<XMLPatchImpl>::Ptr impl(new XMLPatchImpl());
    impl->_patch_id = parsed.patchId;
    impl->_timestamp = str::strtonum<time_t>(parsed.timestamp);
    impl->_category = parsed.category;
    impl->_reboot_needed = parsed.rebootNeeded;
    impl->_affects_pkg_manager = parsed.packageManager;
    // impl._atoms -> std::list<shared_ptr<YUMPatchAtom> > parsed.atoms */

    impl->_summary = parsed.summary;
    impl->_description = parsed.summary;

    impl->_install_notify = parsed.install_notify;
    impl->_delete_notify = parsed.delete_notify;
    impl->_license_to_confirm = parsed.license_to_confirm;
    impl->_vendor = parsed.vendor;
    impl->_size = parsed.size;
    impl->_downloadSize = parsed.downloadSize;
    impl->_install_only = parsed.install_only;
    impl->_build_time = parsed.build_time;
    impl->_install_time = parsed.install_time;

    Arch arch;
    if (!parsed.arch.empty())
      arch = Arch(parsed.arch);

    // Collect basic Resolvable data
    NVRAD dataCollect( parsed.name,
                       Edition( parsed.ver, parsed.rel, parsed.epoch ), arch,
                       createDependencies(parsed, ResTraits<Patch>::kind) );
    Patch::Ptr patch = detail::makeResolvableFromImpl( dataCollect, impl );
    CapFactory _f;

    //MIL << parsed.atoms.size() << " to process" << std::endl;

    // now process the atoms
    ResObject::Ptr atom;
    for (std::list<XMLPatchAtomData_Ptr >::const_iterator it = parsed.atoms.begin(); it != parsed.atoms.end(); it++)
    {
      switch ((*it)->atomType())
      {
        case XMLPatchAtomData::Atom:
        {
          // atoms are mostly used for packages
          // we dont create a package resolvable and then add it to the atoms list, because we
          // dont know if the package is in the pool or not. It is different to Scripts and Messages
          // that are actually contributed to the Patch itself, instead we create and atom, make
          // the patch require the atom, the atom require and freshens the package.

          // get the parsed patch atom data
          XMLPatchAtomData_Ptr atom_data = dynamic_pointer_cast<XMLPatchAtomData>(*it);
          atom = createAtom(*atom_data);
          impl->_atoms.push_back(atom);
          break;
        }
        case XMLPatchAtomData::Message:
        {
          XMLPatchMessageData_Ptr message_data = dynamic_pointer_cast<XMLPatchMessageData>(*it);
          atom = createMessage(*message_data);
          impl->_atoms.push_back(atom);
          break;
        }
        case XMLPatchAtomData::Script:
        {
          XMLPatchScriptData_Ptr script_data = dynamic_pointer_cast<XMLPatchScriptData>(*it);
          atom = createScript(*script_data);
          if ( doesObjectHasFlag( atom, "SCRIPT_EXEC_FAILED" ) )
          {
            WAR << "Patch script not yet successfully executed: " << atom << endl;
          } else {
            impl->_atoms.push_back(atom);
          }
          break;
        }
        default:
          ERR << "Unknown type of atom" << endl;
      }
      // the patch should depends on its atoms, so we inject a requires on the just created atom resolvable
      Capability cap( _f.parse(atom->kind(), atom->name(), Rel::EQ, atom->edition() ));
      patch->injectRequires(cap);
    }
    return patch;
  }
  catch (const Exception & excpt_r)
  {
    ZYPP_CAUGHT(excpt_r);
    Exception nexcpt("Cannot create patch object");
    nexcpt.remember(excpt_r);
    ZYPP_THROW(nexcpt);
  }
  return 0L;
}

Atom::Ptr
XMLFilesBackend::createAtom( const zypp::parser::xmlstore::XMLPatchAtomData & parsed ) const
{
  try
  {
    detail::ResImplTraits<XMLAtomImpl>::Ptr impl(new XMLAtomImpl());

    Arch arch;
    if (!parsed.arch.empty())
      arch = Arch(parsed.arch);

    // Collect basic Resolvable data
    NVRAD dataCollect( parsed.name, Edition( parsed.ver, parsed.rel, parsed.epoch ), arch, createDependencies(parsed, ResTraits<Atom>::kind) );

    impl->_summary = parsed.summary;
    impl->_description = parsed.summary;

    impl->_install_notify = parsed.install_notify;
    impl->_delete_notify = parsed.delete_notify;
    impl->_license_to_confirm = parsed.license_to_confirm;
    impl->_vendor = parsed.vendor;
    impl->_size = parsed.size;
    impl->_downloadSize = parsed.downloadSize;
    impl->_install_only = parsed.install_only;
    impl->_build_time = parsed.build_time;
    impl->_install_time = parsed.install_time;

    Atom::Ptr atom = detail::makeResolvableFromImpl( dataCollect, impl);
    return atom;
  }
  catch (const Exception & excpt_r)
  {
    ZYPP_CAUGHT(excpt_r);
    Exception nexcpt("Cannot create atom object");
    nexcpt.remember(excpt_r);
    ZYPP_THROW(nexcpt);
  }
  return 0L;
}

Message::Ptr
XMLFilesBackend::createMessage( const zypp::parser::xmlstore::XMLPatchMessageData & parsed ) const
{
  try
  {
    detail::ResImplTraits<XMLMessageImpl>::Ptr impl(new XMLMessageImpl());
    impl->_text = parsed.text;

    Arch arch;
    if (!parsed.arch.empty())
      arch = Arch(parsed.arch);

    impl->_summary = parsed.summary;
    impl->_description = parsed.summary;

    impl->_install_notify = parsed.install_notify;
    impl->_delete_notify = parsed.delete_notify;
    impl->_license_to_confirm = parsed.license_to_confirm;
    impl->_vendor = parsed.vendor;
    impl->_size = parsed.size;
    impl->_downloadSize = parsed.downloadSize;
    impl->_install_only = parsed.install_only;
    impl->_build_time = parsed.build_time;
    impl->_install_time = parsed.install_time;

    // Collect basic Resolvable data
    NVRAD dataCollect( parsed.name, Edition( parsed.ver, parsed.rel, parsed.epoch ), arch, createDependencies(parsed, ResTraits<Message>::kind) );
    Message::Ptr message = detail::makeResolvableFromImpl( dataCollect, impl);
    return message;
  }
  catch (const Exception & excpt_r)
  {
    ZYPP_CAUGHT(excpt_r);
    Exception nexcpt("Cannot create message object");
    nexcpt.remember(excpt_r);
    ZYPP_THROW(nexcpt);
  }
  return 0L;
}

Script::Ptr
XMLFilesBackend::createScript(const zypp::parser::xmlstore::XMLPatchScriptData & parsed ) const
{
  try
  {
    detail::ResImplTraits<XMLScriptImpl>::Ptr impl(new XMLScriptImpl());

    impl->_doScript = parsed.doScript;
    impl->_undoScript = parsed.undoScript;

    Arch arch;
    if (!parsed.arch.empty())
      arch = Arch(parsed.arch);

    impl->_summary = parsed.summary;
    impl->_description = parsed.summary;

    impl->_install_notify = parsed.install_notify;
    impl->_delete_notify = parsed.delete_notify;
    impl->_license_to_confirm = parsed.license_to_confirm;
    impl->_vendor = parsed.vendor;
    impl->_size = parsed.size;
    impl->_downloadSize = parsed.downloadSize;
    impl->_install_only = parsed.install_only;
    impl->_build_time = parsed.build_time;
    impl->_install_time = parsed.install_time;

    // Collect basic Resolvable data
    NVRAD dataCollect( parsed.name, Edition( parsed.ver, parsed.rel, parsed.epoch ), arch, createDependencies(parsed, ResTraits<Script>::kind));
    Script::Ptr script = detail::makeResolvableFromImpl( dataCollect, impl );
    return script;
  }
  catch (const Exception & excpt_r)
  {
    ZYPP_CAUGHT(excpt_r);
    Exception nexcpt("Cannot create script object");
    nexcpt.remember(excpt_r);
    ZYPP_THROW(nexcpt);
  }
  catch (const std::exception & excpt_r)
  {
    ERR << excpt_r.what() << endl;
    ZYPP_THROW(Exception("Cannot create script object"));
  }
  return 0L;
}

Language::Ptr
XMLFilesBackend::createLanguage( const zypp::parser::xmlstore::XMLLanguageData & parsed ) const
{
  try
  {
    return Language::installedInstance( Locale(parsed.name) );
  }
  catch (const Exception & excpt_r)
  {
    ZYPP_CAUGHT(excpt_r);
    Exception nexcpt("Cannot create language object");
    nexcpt.remember(excpt_r);
    ZYPP_THROW(nexcpt);
  }
  return 0L;
}


Product::Ptr
XMLFilesBackend::createProduct( const zypp::parser::xmlstore::XMLProductData & parsed ) const
{
  try
  {
    detail::ResImplTraits<XMLProductImpl>::Ptr impl(new XMLProductImpl());

    Edition parser_edition = ( parsed.parser_version.empty() ? Edition::noedition : Edition(parsed.parser_version) );

    impl->_summary = parsed.summary;
    impl->_description = parsed.summary;

    impl->_install_notify = parsed.install_notify;
    impl->_delete_notify = parsed.delete_notify;
    impl->_license_to_confirm = parsed.license_to_confirm;
    impl->_vendor = parsed.vendor;
    impl->_size = parsed.size;
    impl->_downloadSize = parsed.downloadSize;
    impl->_install_only = parsed.install_only;
    impl->_build_time = parsed.build_time;
    impl->_install_time = parsed.install_time;

    impl->_type = parsed.type;
    impl->_short_name = parsed.short_name;
    impl->_dist_name = parsed.dist_name;
    impl->_dist_version = parsed.dist_version;

    if ( parsed.releasenotesurl.size() > 0 )
      impl->_release_notes_url = parsed.releasenotesurl;
    else
      impl->_release_notes_url = Url();

    // update_urls
    list<string> update_urls = parsed.update_urls;
    for ( list<string>::const_iterator it = update_urls.begin(); it != update_urls.end(); ++it )
    {
      try
      {
        Url u(*it);
        impl->_update_urls.push_back (u);
      }
      catch ( const Exception &e )
      {
	ZYPP_CAUGHT(e);
	Exception ne("Error parsing update url: " + e.msg());
	ne.remember(e);
	ZYPP_THROW(ne);
      }
    }

    // extra_urls
    list<string> extra_urls = parsed.extra_urls;
    for ( list<string>::const_iterator it = extra_urls.begin(); it != extra_urls.end(); ++it )
    {
      try
      {
        Url u(*it);
        impl->_extra_urls.push_back (u);
      }
      catch ( const Exception &e )
      {
	ZYPP_CAUGHT(e);
	Exception ne("Error parsing extra url: " + e.msg());
	ne.remember(e);
	ZYPP_THROW(ne);
      }
    }

    // extra_urls
    list<string> optional_urls = parsed.optional_urls;
    for ( list<string>::const_iterator it = optional_urls.begin(); it != optional_urls.end(); ++it )
    {
      try
      {
        Url u(*it);
        impl->_optional_urls.push_back (u);
      }
      catch ( const Exception &e )
      {
	ZYPP_CAUGHT(e);
	Exception ne("Error parsing optional url: " + e.msg());
	ne.remember(e);
	ZYPP_THROW(ne);
      }
    }

    impl->_flags = parsed.flags;

    Arch arch;
    if (!parsed.arch.empty())
      arch = Arch(parsed.arch);

    Edition prod_edition( parsed.ver, parsed.rel, parsed.epoch );
    string prod_name(parsed.name);
    // check for product name to see if it was written with distname
    // as name as part of https://bugzilla.novell.com/show_bug.cgi?id=205392
    bool save_new_product_again_workaround = false;
    if ( parser_edition == Edition::noedition )
    {
      MIL << "Product " << parsed.name << " " << prod_edition << " possibly suffers from bug #205392. checking..." << endl;
      PRODUCT_TABLE_ENTRY *all_products = products_table();
      while ( all_products->dist_name != 0L )
      {
        //MIL << "Checking " << parsed.name << " " << prod_edition << " with " << all_products->dist_name << " " << all_products->dist_version << endl;
        if ( ( parsed.name == all_products->dist_name ) && ( prod_edition.asString() == all_products->dist_version ) )
        {
          MIL << "[ATTENTION] Detected bug #205392. Product " << parsed.name << " " << prod_edition << " will be changed to " << all_products->product_name << " " << all_products->product_version << std::endl;

          // save pathname of the old wrong product
          Pathname wrong_product = Pathname(dirForResolvableKind(ResTraits<zypp::Product>::kind)) + fileNameForNVR( NVR( parsed.name, prod_edition) );

          // ok, potentially this is a wrong product, well, IT IS!
          // overwrte those here as they are used in dataCollect
          prod_name = string(all_products->product_name);
          prod_edition = Edition(all_products->product_version);

          // those were already set, so reset them.
          impl->_dist_name = all_products->dist_name;
          impl->_dist_version = Edition(all_products->dist_version);

          // ok, now mark for save this product and delete the old one
          deleteFileObject( wrong_product );
          MIL << "Fix for bug #205392 Old product deleted." << std::endl;
          save_new_product_again_workaround = true;
          break;
        }
        ++all_products;
      }

    }

    // replace spaces to underscores
    std::replace(prod_name.begin(), prod_name.end(), ' ', '_');

    // Collect basic Resolvable data
    NVRAD dataCollect( prod_name, prod_edition, arch, createDependencies(parsed, ResTraits<Product>::kind) );
    Product::Ptr product = detail::makeResolvableFromImpl( dataCollect, impl );

    if ( save_new_product_again_workaround )
    {
      const_cast<XMLFilesBackend *>(this)->storeObject(product);
      MIL << "Fixed Product saved. Fix for bug #205392. complete" << std::endl;
    }

    return product;
  }
  catch (const Exception & excpt_r)
  {
    ZYPP_CAUGHT(excpt_r);
    Exception nexcpt("Cannot create product object");
    nexcpt.remember(excpt_r);
    ZYPP_THROW(nexcpt);
  }
  return 0L;
}

Pattern::Ptr
XMLFilesBackend::createPattern( const zypp::parser::xmlstore::XMLPatternData & parsed ) const
{
  try
  {
    detail::ResImplTraits<XMLPatternImpl>::Ptr impl(new XMLPatternImpl());

    impl->_summary = parsed.summary;
    impl->_description = parsed.summary;

    impl->_install_notify = parsed.install_notify;
    impl->_delete_notify = parsed.delete_notify;
    impl->_license_to_confirm = parsed.license_to_confirm;
    impl->_vendor = parsed.vendor;
    impl->_size = parsed.size;
    impl->_downloadSize = parsed.downloadSize;
    impl->_install_only = parsed.install_only;
    impl->_build_time = parsed.build_time;
    impl->_install_time = parsed.install_time;

    impl->_user_visible = parsed.userVisible;
    impl->_default = ((parsed.default_ == "false" ) ? false : true );
    impl->_category = parsed.category;
    impl->_icon = parsed.icon;
    impl->_script = parsed.script;

    Arch arch;
    if (!parsed.arch.empty())
      arch = Arch(parsed.arch);

    // Workaround for bug #307743:
    // Hijack installed patterns with empty vendor.
    if ( impl->_vendor.empty() )
    {
      impl->_vendor = "SUSE (assumed)";
    }

    // Collect basic Resolvable data
    NVRAD dataCollect( parsed.name, Edition( parsed.ver, parsed.rel, parsed.epoch ), arch, createDependencies( parsed, ResTraits<Pattern>::kind));
    Pattern::Ptr pattern = detail::makeResolvableFromImpl( dataCollect, impl );
    return pattern;
  }
  catch (const Exception & excpt_r)
  {
    ZYPP_CAUGHT(excpt_r);
    Exception nexcpt("Cannot create installation pattern object");
    nexcpt.remember(excpt_r);
    ZYPP_THROW(nexcpt);
  }
  return 0L;
}

Selection::Ptr
XMLFilesBackend::createSelection( const zypp::parser::xmlstore::XMLPatternData & parsed ) const
{
  try
  {
    detail::ResImplTraits<XMLSelectionImpl>::Ptr impl(new XMLSelectionImpl());

    impl->_summary = parsed.summary;
    impl->_description = parsed.summary;

    impl->_install_notify = parsed.install_notify;
    impl->_delete_notify = parsed.delete_notify;
    impl->_license_to_confirm = parsed.license_to_confirm;
    impl->_vendor = parsed.vendor;
    impl->_size = parsed.size;
    impl->_downloadSize = parsed.downloadSize;
    impl->_install_only = parsed.install_only;
    impl->_build_time = parsed.build_time;
    impl->_install_time = parsed.install_time;

    impl->_visible = parsed.userVisible;
    impl->_name = parsed.name;
    //impl->_default = ((parsed.default_ == "false" ) ? false : true );
    impl->_category = parsed.category;

    Arch arch;
    if (!parsed.arch.empty())
      arch = Arch(parsed.arch);

    // Collect basic Resolvable data
    NVRAD dataCollect( parsed.name, Edition( parsed.ver, parsed.rel, parsed.epoch ), arch, createDependencies( parsed, ResTraits<Pattern>::kind));
    Selection::Ptr selection = detail::makeResolvableFromImpl( dataCollect, impl );
    return selection;
  }
  catch (const Exception & excpt_r)
  {
    ZYPP_CAUGHT(excpt_r);
    Exception nexcpt("Cannot create installation selection object");
    nexcpt.remember(excpt_r);
    ZYPP_THROW(nexcpt);
  }
  return 0L;
}

Dependencies
XMLFilesBackend::createDependencies( const zypp::parser::xmlstore::XMLResObjectData & parsed, const Resolvable::Kind my_kind ) const
{
  Dependencies _deps;
  for (std::list<XMLDependency>::const_iterator it = parsed.provides.begin(); it != parsed.provides.end(); it++)
  {
    _deps[Dep::PROVIDES].insert(createCapability(*it, my_kind));
  }
  for (std::list<XMLDependency>::const_iterator it = parsed.conflicts.begin(); it != parsed.conflicts.end(); it++)
  {
    _deps[Dep::CONFLICTS].insert(createCapability(*it, my_kind));
  }

  for (std::list<XMLDependency>::const_iterator it = parsed.obsoletes.begin(); it != parsed.obsoletes.end(); it++)
  {
    _deps[Dep::OBSOLETES].insert(createCapability(*it, my_kind));
  }

  for (std::list<XMLDependency>::const_iterator it = parsed.freshens.begin(); it != parsed.freshens.end(); it++)
  {
    _deps[Dep::FRESHENS].insert(createCapability(*it, my_kind));
  }

  for (std::list<XMLDependency>::const_iterator it = parsed.recommends.begin(); it != parsed.recommends.end(); it++)
  {
    _deps[Dep::RECOMMENDS].insert(createCapability(*it, my_kind));
  }

  for (std::list<XMLDependency>::const_iterator it = parsed.suggests.begin(); it != parsed.suggests.end(); it++)
  {
    _deps[Dep::SUGGESTS].insert(createCapability(*it, my_kind));
  }

  for (std::list<XMLDependency>::const_iterator it = parsed.enhances.begin(); it != parsed.enhances.end(); it++)
  {
    _deps[Dep::ENHANCES].insert(createCapability(*it, my_kind));
  }

  for (std::list<XMLDependency>::const_iterator it = parsed.requires.begin(); it != parsed.requires.end(); it++)
  {
    _deps[Dep::REQUIRES].insert(createCapability(*it, my_kind));
  }

  for (std::list<XMLDependency>::const_iterator it = parsed.prerequires.begin(); it != parsed.prerequires.end(); it++)
  {
    _deps[Dep::PREREQUIRES].insert(createCapability(*it, my_kind));
  }
  return _deps;
}

Capability
XMLFilesBackend::createCapability(const XMLDependency & dep, const Resolvable::Kind & my_kind) const
{
  CapFactory _f;
  Resolvable::Kind _kind = dep.kind == "" ? my_kind : Resolvable::Kind(dep.kind);
  Capability cap;
  cap = _f.parse( _kind, dep.encoded );
  return cap;
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
} // namespace storage
///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
