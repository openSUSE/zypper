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
#include <cstdio>
#include <fcntl.h>

#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"

#include "zypp/CapFactory.h"
#include "zypp/Digest.h"
#include "zypp/Source.h"
#include "zypp/SourceManager.h"
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

#include "zypp/parser/xmlstore/XMLSourceCacheParser.h"

#include "boost/filesystem/operations.hpp" // includes boost/filesystem/path.hpp
#include "boost/filesystem/fstream.hpp"    // ditto

#include "XMLFilesBackend.h"
#include "serialize.h"

//#define ZYPP_DB_DIR "/var/lib/zypp_db/"
#define ZYPP_DB_DIR ( getZYpp()->homePath().asString()+"/db/" )

using std::endl;
using namespace boost::filesystem;
using namespace zypp;
using namespace zypp::filesystem;

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
  //d->kinds.insert(ResTraits<zypp::Script>::kind);
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
    bool isthere = exists(dirForResolvableKind(kind));
    ok = ok && isthere;
  }

  // resolvable flags folder flags
  for ( it_kinds = d->kinds_flags.begin() ; it_kinds != d->kinds_flags.end(); ++it_kinds )
  {
    Resolvable::Kind kind = (*it_kinds);
    bool isthere = exists(dirForResolvableKindFlags(kind));
    ok = ok && isthere;
  }

  // named flags
  bool nmthere = exists(dirForNamedFlags());
  ok = ok && nmthere;
    
  Pathname sourcesdir = d->root + ZYPP_DB_DIR + "/sources";
  bool srcthere = PathInfo(sourcesdir).isExist();
  ok = ok && srcthere;
  
  if (srcthere && fixperms)
  {
    MIL << "Making " << sourcesdir << " not readable by others (0700)" << std::endl;
    filesystem::chmod( sourcesdir, 0700);
  }
  
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

  // create dir for source list
  Pathname source_p = d->root + Pathname(ZYPP_DB_DIR) + Pathname("/sources/");
  if (0 != assert_dir(source_p, 0700))
    ZYPP_THROW(Exception("Cannot create directory " + source_p.asString()));
  else
    MIL << "Created " << source_p.asString() << std::endl;
}

void XMLFilesBackend::setRandomFileNameEnabled( bool enabled )
{
  d->randomFileName = enabled;
}

std::string
XMLFilesBackend::dirForResolvableKind( Resolvable::Kind kind ) const
{
  std::string dir;
  dir += Pathname( d->root + Pathname(ZYPP_DB_DIR) + Pathname(resolvableKindToString(kind, true)) ).asString();
  return dir;
}

std::string
XMLFilesBackend::dirForResolvableKindFlags( Resolvable::Kind kind ) const
{
  std::string dir;
  dir += Pathname( d->root + Pathname(ZYPP_DB_DIR) + Pathname("flags") + Pathname(resolvableKindToString(kind, true)) ).asString();
  return dir;
}

std::string
XMLFilesBackend::dirForNamedFlags() const
{
  std::string dir;
  dir += Pathname( d->root + Pathname(ZYPP_DB_DIR) + Pathname("named-flags")).asString();
  return dir;
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
XMLFilesBackend::fileNameForResolvable( ResObject::constPtr resolvable ) const
{
  std::string filename;
  //filename = d->randomFileName ? randomString(40) : (resolvable->name() + suffix);
  filename = resolvable->name();
  if (resolvable->edition() != Edition::noedition )
  {
     filename += "-" + resolvable->edition().asString();
  }
  // get rid of spaces and other characters
  std::stringstream filename_stream(filename);
  std::string filename_encoded = Digest::digest("MD5", filename_stream);
  return filename_encoded;
}

std::string
XMLFilesBackend::fullPathForResolvable( ResObject::constPtr resolvable ) const
{
  return path( path(dirForResolvable(resolvable)) / path(fileNameForResolvable(resolvable))).string();
}

std::string
XMLFilesBackend::fullPathForNamedFlags( const std::string &key ) const
{
  std::stringstream key_stream(key);
  std::string key_encoded = Digest::digest("MD5", key_stream);
  return path( path(dirForNamedFlags()) / path(key_encoded)).string();
}

std::string
XMLFilesBackend::fullPathForResolvableFlags( ResObject::constPtr resolvable ) const
{
  // flags are in a hidden file with the same name
  return path( path(dirForResolvableFlags(resolvable)) / path(fileNameForResolvable(resolvable))).string();
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
    ZYPP_THROW (Exception( "Can't write flags to store") );
  }
  updateTimestamp();
}

std::set<std::string>
XMLFilesBackend::flagsFromFile( const std::string &filename ) const
{
  std::set<std::string> _flags;
  // do we have previous saved flags?
  if (!exists(path(filename)))
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
XMLFilesBackend::deleteObject( ResObject::constPtr resolvable )
{
  // only remove the file
  std::string filename = fullPathForResolvable(resolvable);
  try
  {
    int ret = filesystem::unlink(Pathname(filename));
    if ( ret != 0 )
    {
      ERR << "Error removing resolvable " << resolvable << std::endl;
      ZYPP_THROW(Exception("Error deleting " + filename));
    }
    updateTimestamp();
  }
  catch(std::exception &e)
  {
    ERR << "Error removing resolvable " << resolvable << std::endl;
    ZYPP_THROW(Exception(e.what()));
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
    catch (const Exception & excpt_r) {
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
  std::string dir_path = dirForResolvableKind(kind);
  DBG << "Reading objects of kind " << resolvableKindToString(kind) << " in " << dir_path << std::endl;
  directory_iterator end_iter;
  // return empty list if the dir does not exist
  if ( !exists( dir_path ) )
  {
    ERR << "path " << dir_path << " does not exists. Required to read objects of kind " << resolvableKindToString(kind) << std::endl;
    return std::list<ResObject::Ptr>();
  }

  for ( directory_iterator dir_itr( dir_path ); dir_itr != end_iter; ++dir_itr )
  {
    DBG << "[" << resolvableKindToString( kind, false ) << "] - " << dir_itr->leaf() << std::endl;
    std::list<ResObject::Ptr> objects_for_file;
    objects_for_file = resolvablesFromFile( dir_path + "/" + dir_itr->leaf(), kind);
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
    impl->_size = parsed.size;
    impl->_archive_size = parsed.archive_size;
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
          impl->_atoms.push_back(atom);
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
    ZYPP_THROW(Exception("Cannot create patch object"));
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
    impl->_size = parsed.size;
    impl->_archive_size = parsed.archive_size;
    impl->_install_only = parsed.install_only;
    impl->_build_time = parsed.build_time;
    impl->_install_time = parsed.install_time;
    
    Atom::Ptr atom = detail::makeResolvableFromImpl( dataCollect, impl);
    return atom;
  }
  catch (const Exception & excpt_r)
  {
    ZYPP_CAUGHT(excpt_r);
    ZYPP_THROW(Exception("Cannot create atom object"));
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
    impl->_size = parsed.size;
    impl->_archive_size = parsed.archive_size;
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
    ZYPP_THROW(Exception("Cannot create message object"));
  }
  return 0L;
}

Script::Ptr
XMLFilesBackend::createScript(const zypp::parser::xmlstore::XMLPatchScriptData & parsed ) const
{
  try
  {
    detail::ResImplTraits<XMLScriptImpl>::Ptr impl(new XMLScriptImpl());

    ofstream file;
    file.open(impl->_do_script.path().asString().c_str());
    file << parsed.do_script;;
    file.close();

    file.open(impl->_undo_script.path().asString().c_str());
    file << parsed.undo_script;;
    file.close();

    Arch arch;
    if (!parsed.arch.empty())
      arch = Arch(parsed.arch);
    
    impl->_summary = parsed.summary;
    impl->_description = parsed.summary;

    impl->_install_notify = parsed.install_notify;
    impl->_delete_notify = parsed.delete_notify;
    impl->_license_to_confirm = parsed.license_to_confirm;
    impl->_size = parsed.size;
    impl->_archive_size = parsed.archive_size;
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
    ZYPP_THROW(Exception("Cannot create script object"));
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
    ZYPP_THROW(Exception("Cannot create language object"));
  }
  return 0L;
}


Product::Ptr
XMLFilesBackend::createProduct( const zypp::parser::xmlstore::XMLProductData & parsed ) const
{
  try
  {
    detail::ResImplTraits<XMLProductImpl>::Ptr impl(new XMLProductImpl());

    impl->_summary = parsed.summary;
    impl->_description = parsed.summary;

    impl->_install_notify = parsed.install_notify;
    impl->_delete_notify = parsed.delete_notify;
    impl->_license_to_confirm = parsed.license_to_confirm;
    impl->_size = parsed.size;
    impl->_archive_size = parsed.archive_size;
    impl->_install_only = parsed.install_only;
    impl->_build_time = parsed.build_time;
    impl->_install_time = parsed.install_time;
    
    impl->_category = parsed.type;
    impl->_short_name = parsed.short_name;

    if ( parsed.releasenotesurl.size() > 0 )
      impl->_release_notes_url = parsed.releasenotesurl;
    else
      impl->_release_notes_url = Url();

    // update_urls
    std::list<std::string>::const_iterator
	b = parsed.update_urls.begin(),
	e = parsed.update_urls.end(),
	i;
    for (i = b; i != e; ++i) {
	Url u;
	try {
	    u = *i;
	}
	catch (...) {
	}
	impl->_update_urls.push_back (u);
    }

    impl->_flags = parsed.flags;

    Arch arch;
    if (!parsed.arch.empty())
      arch = Arch(parsed.arch);

    // Collect basic Resolvable data
    NVRAD dataCollect( parsed.name, Edition( parsed.ver, parsed.rel, parsed.epoch ), arch, createDependencies(parsed, ResTraits<Product>::kind) );
    Product::Ptr product = detail::makeResolvableFromImpl( dataCollect, impl );
    return product;
  }
  catch (const Exception & excpt_r)
  {
    ZYPP_CAUGHT(excpt_r);
    ZYPP_THROW(Exception("Cannot create product object"));
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
    impl->_size = parsed.size;
    impl->_archive_size = parsed.archive_size;
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

    // Collect basic Resolvable data
    NVRAD dataCollect( parsed.name, Edition( parsed.ver, parsed.rel, parsed.epoch ), arch, createDependencies( parsed, ResTraits<Pattern>::kind));
    Pattern::Ptr pattern = detail::makeResolvableFromImpl( dataCollect, impl );
    return pattern;
  }
  catch (const Exception & excpt_r)
  {
    ZYPP_CAUGHT(excpt_r);
    ZYPP_THROW(Exception("Cannot create installation pattern object"));
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
    impl->_size = parsed.size;
    impl->_archive_size = parsed.archive_size;
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
    ZYPP_THROW(Exception("Cannot create installation selection object"));
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

/////////////////////////////////////////////////////////
// SOURCES API
////////////////////////////////////////////////////////

std::list<PersistentStorage::SourceData>
XMLFilesBackend::storedSources() const
{
  path source_p = path(d->root.asString()) / path(ZYPP_DB_DIR) / path ("sources");
  std::list<PersistentStorage::SourceData> sources;
  DBG << "Reading source cache in " << source_p.string() << std::endl;
  directory_iterator end_iter;
  // return empty list if the dir does not exist
  if ( !exists( source_p ) )
  {
    ERR << "path " << source_p.string() << " does not exists. Required to read source cache " << std::endl;
    return std::list<PersistentStorage::SourceData>();
  }

  for ( directory_iterator dir_itr( source_p ); dir_itr != end_iter; ++dir_itr )
  {
    DBG << "[source-list] - " << dir_itr->leaf() << std::endl;
    //sources.insert( sourceDataFromCacheFile( source_p + "/" + dir_itr->leaf() ) );
    std::string full_path = (source_p / dir_itr->leaf()).string();
    std::ifstream anIstream(full_path.c_str());
    zypp::parser::xmlstore::XMLSourceCacheParser iter(anIstream, "");
    for (; ! iter.atEnd(); ++iter) {
      PersistentStorage::SourceData data = **iter;
      sources.push_back(data);
    }
  }
  MIL << "done reading source cache" << std::endl;
  return sources;

}

void
XMLFilesBackend::storeSource(const PersistentStorage::SourceData &data)
{
  // serialize and save a file
  std::string xml = toXML(data);
  path source_p = path(d->root.asString()) / path(ZYPP_DB_DIR) / path ("sources");

  // generate a filename
  if (data.alias.size() == 0)
  {
    ZYPP_THROW(Exception("Cant save source with empty alias"));
  }

  //DBG << std::endl << xml << std::endl;
  std::ofstream file;
  //DBG << filename << std::endl;
  try
  {
    std::stringstream message_stream(data.alias);
    std::string full_path = (source_p / Digest::digest("MD5", message_stream)).string();

    file.open(full_path.c_str());
    file << xml;
    file.close();
  }
  catch ( std::exception &e )
  {
    ERR << "Error saving source " << data.alias << " in the cache" << std::endl;
    ZYPP_THROW(Exception(e.what()));
  }
  updateTimestamp();
}

void
XMLFilesBackend::deleteSource(const std::string &alias)
{
  // just delete the files
  path source_p = path(d->root.asString()) / path(ZYPP_DB_DIR) / path ("sources");
  try
  {
    std::stringstream message_stream(alias);
    std::string full_path = (source_p / Digest::digest("MD5", message_stream)).string();
    remove(full_path);
  }
  catch ( std::exception &e )
  {
    ERR << "Error deleting source " << alias << " in the cache" << std::endl;
    ZYPP_THROW(Exception(e.what()));
  }
  updateTimestamp();
}

/////////////////////////////////////////////////////////////////
} // namespace storage
///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
