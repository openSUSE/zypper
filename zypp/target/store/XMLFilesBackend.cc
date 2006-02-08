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

#include "zypp/target/store/xml/XMLPatchImpl.h"
#include "zypp/target/store/xml/XMLMessageImpl.h"
#include "zypp/target/store/xml/XMLScriptImpl.h"
#include "zypp/target/store/xml/XMLSelectionImpl.h"
#include "zypp/target/store/xml/XMLProductImpl.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <streambuf>

#include <list>

#include <zypp/target/store/xml/XMLSourceCacheParser.h>

#include "boost/filesystem/operations.hpp" // includes boost/filesystem/path.hpp
#include "boost/filesystem/fstream.hpp"    // ditto

#include "XMLFilesBackend.h"
#include "serialize.h"

#include "md5.h"

#define ZYPP_DB_DIR "/var/lib/zypp_db"

using std::endl;
using namespace boost::filesystem;
using namespace zypp;
using namespace zypp::parser::yum;

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
XMLFilesBackend::isBackendInitialized() const
{
  bool ok = true;
  ok = ok && exists( ZYPP_DB_DIR );
  std::set<Resolvable::Kind>::const_iterator it_kinds;
  for ( it_kinds = d->kinds.begin() ; it_kinds != d->kinds.end(); ++it_kinds )
  {
    Resolvable::Kind kind = (*it_kinds);
    ok = ok && exists(dirForResolvableKind(kind));
  }
  ok = ok && exists( path(d->root.asString()) / path(ZYPP_DB_DIR) / path ("source-cache") );
  return ok;
}

void
XMLFilesBackend::initBackend()
{
  // FIXME duncan * handle exceptions
  DBG << "Creating directory structure..." << std::endl;
  try
  {
    path topdir = path(d->root.asString()) / path(ZYPP_DB_DIR);
    if (!exists(topdir))
      create_directory(topdir);
    MIL << "Created..." << topdir.string() << std::endl;
    std::set<Resolvable::Kind>::const_iterator it_kinds;
    for ( it_kinds = d->kinds.begin() ; it_kinds != d->kinds.end(); ++it_kinds )
    {
      Resolvable::Kind kind = (*it_kinds);
      # warning "add exception handling here"
      path p(topdir / path(resolvableKindToString(kind, true /* plural */)));
      if (!exists(p))
      {
        create_directory(p);
        MIL << "Created..." << p.string() << std::endl;
      }
    }
    // create source-cache
    path source_p = path(d->root.asString()) / path(ZYPP_DB_DIR) / path ("source-cache");
    if (!exists(source_p))
    {
      create_directory(source_p);
      MIL << "Created..." << source_p.string() << std::endl;
    }
  }
  catch(std::exception &e)
  {
    ZYPP_RETHROW(Exception(e.what()));
  }
}

void XMLFilesBackend::setRandomFileNameEnabled( bool enabled )
{
  d->randomFileName = enabled;
}

std::string
XMLFilesBackend::dirForResolvableKind( Resolvable::Kind kind ) const
{
  std::string dir;
  dir += path( path(d->root.asString()) / path(ZYPP_DB_DIR) / path(resolvableKindToString(kind, true)) ).string();
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
  // only append edition if there is one
  std::string suffix = ( (resolvable->edition() == Edition::noedition) ? std::string() : ("-" + resolvable->edition().version() + "-" + resolvable->edition().release()) );
  filename = d->randomFileName ? randomString(40) : (resolvable->name() + suffix);
  return path( path(dirForResolvable(resolvable)) / path(filename)).string();
}

void
XMLFilesBackend::storeObject( Resolvable::constPtr resolvable )
{
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
      DBG << "here..." << std::endl;
      resolvable = createPatch(**iter);
      break;
    }
  }
  else if ( kind == ResTraits<zypp::Product>::kind )
  {
    YUMProductParser iter(res_file,"");
    for (; !iter.atEnd(); ++iter)
    {
      DBG << "here..." << std::endl;
      resolvable = createProduct(**iter);
      break;
    }
  }
  else if ( kind == ResTraits<zypp::Selection>::kind )
  {
    YUMGroupParser iter(res_file,"");
    for (; !iter.atEnd(); ++iter)
    {
      DBG << "here..." << std::endl;
      resolvable = createSelection(**iter);
      break;
    }
  }
  /*
  else if ( kind == ResTraits<zypp::Message>::kind )
  {
    YUMMessageParser iter(res_file,"");
    for (; !iter.atEnd(); ++iter)
    {
      DBG << "here..." << std::endl;
      resolvable = createMessage(**iter);
      break;
    }
  }
  else if ( kind == ResTraits<zypp::Script>::kind )
  {
    YUMScriptParser iter(res_file,"");
    for (; !iter.atEnd(); ++iter)
    {
      DBG << "here..." << std::endl;
      resolvable = createScript(**iter);
      break;
    }
  }
  */
  else
  {
    resolvable = 0;
  }
  return resolvable;
}

std::list<Resolvable::Ptr>
XMLFilesBackend::storedObjects() const
{
  DBG << std::endl;
  std::list<Resolvable::Ptr> objects;

  std::set<Resolvable::Kind>::const_iterator it_kinds;
  for ( it_kinds = d->kinds.begin() ; it_kinds != d->kinds.end(); ++it_kinds )
  {
    Resolvable::Kind kind = (*it_kinds);
    std::list<Resolvable::Ptr> objects_for_kind = storedObjects(kind);
    std::list<Resolvable::Ptr>::iterator it;
    for( it = objects_for_kind.begin(); it != objects_for_kind.end(); ++it)
    {
      //DBG << "adding objects back" << std::endl;
      objects.push_back(*it);
    }
  }
  return objects;
}

std::list<Resolvable::Ptr>
XMLFilesBackend::storedObjects(const Resolvable::Kind kind) const
{
  std::list<Resolvable::Ptr> objects;
  std::string dir_path = dirForResolvableKind(kind);
  DBG << "Reading objects of kind " << resolvableKindToString(kind) << " in " << dir_path << std::endl;
  directory_iterator end_iter;
  // return empty list if the dir does not exist
  if ( !exists( dir_path ) )
  {
    ERR << "path " << dir_path << " does not exists. Required to read objects of kind " << resolvableKindToString(kind) << std::endl;
    return std::list<Resolvable::Ptr>();
  }

  for ( directory_iterator dir_itr( dir_path ); dir_itr != end_iter; ++dir_itr )
  {
    DBG << "[" << resolvableKindToString( kind, false ) << "] - " << dir_itr->leaf() << std::endl;
    objects.push_back( resolvableFromFile( dir_path + "/" + dir_itr->leaf(), kind) );
  }
  MIL << "done reading stored objecs of kind " << resolvableKindToString(kind) << std::endl;
  return objects;
}

std::list<Resolvable::Ptr>
XMLFilesBackend::storedObjects(const Resolvable::Kind kind, const std::string & name, bool partial_match) const
{
  std::list<Resolvable::Ptr> result;
  std::list<Resolvable::Ptr> all;
  all = storedObjects(kind);
  std::list<Resolvable::Ptr>::const_iterator it;
  for( it = all.begin(); it != all.end(); ++it)
  {
    Resolvable::Ptr item = *it;
    if (item->name() == name )
      result.push_back(item);
  }
  MIL << "done reading stored objecs of kind " << resolvableKindToString(kind) << " and keyword [" << name <<"]" << std::endl;
  return result;
}

Patch::Ptr
XMLFilesBackend::createPatch( const zypp::parser::yum::YUMPatchData & parsed ) const
{
  try
  {
    shared_ptr<XMLPatchImpl> impl(new XMLPatchImpl());
    impl->_patch_id = parsed.patchId;
    impl->_timestamp = str::strtonum<time_t>(parsed.timestamp);
    impl->_category = parsed.category;
    impl->_reboot_needed = parsed.rebootNeeded;
    impl->_affects_pkg_manager = parsed.packageManager;
    // impl._atoms -> std::list<shared_ptr<YUMPatchAtom> > parsed.atoms */
    
    impl->_summary = parsed.summary;
    impl->_description = parsed.summary;
    
    // Collect basic Resolvable data
    NVRAD dataCollect( parsed.name,
                       Edition( parsed.ver, parsed.rel, parsed.epoch ), Arch_noarch,
                       createDependencies(parsed, ResTraits<Patch>::kind) );
    Patch::Ptr patch = detail::makeResolvableFromImpl( dataCollect, impl );
    return patch;
  }
  catch (const Exception & excpt_r)
  {
    ERR << excpt_r << endl;
    throw "Cannot create patch object";
  }
}

Message::Ptr
XMLFilesBackend::createMessage( const zypp::parser::yum::YUMPatchMessage & parsed ) const
{
  try
  {
    shared_ptr<XMLMessageImpl> impl(new XMLMessageImpl());
    impl->_text = parsed.text;

    // Collect basic Resolvable data
    NVRAD dataCollect( parsed.name, Edition( parsed.ver, parsed.rel, parsed.epoch ), Arch_noarch, createDependencies(parsed, ResTraits<Message>::kind) );
    Message::Ptr message = detail::makeResolvableFromImpl( dataCollect, impl);
    return message;
  }
  catch (const Exception & excpt_r)
  {
    ERR << excpt_r << endl;
    throw "Cannot create message object";
  }
}

Script::Ptr
XMLFilesBackend::createScript(const zypp::parser::yum::YUMPatchScript & parsed ) const
{
  try
  {
    shared_ptr<XMLScriptImpl> impl(new XMLScriptImpl());
    impl->_do_script = parsed.do_script;
    impl->_undo_script = parsed.undo_script;
    // Collect basic Resolvable data
    NVRAD dataCollect( parsed.name, Edition( parsed.ver, parsed.rel, parsed.epoch ), Arch_noarch, createDependencies(parsed, ResTraits<Script>::kind));
    Script::Ptr script = detail::makeResolvableFromImpl( dataCollect, impl );
    return script;
  }
  catch (const Exception & excpt_r)
  {
    ERR << excpt_r << endl;
    throw "Cannot create script object";
  }
}

/*
      std::string groupId;
      std::list<MultiLang> name;
      std::string default_;
      std::string userVisible;
      std::list<MultiLang> description;
      std::list<MetaPkg> grouplist;
      std::list<PackageReq> package_list;
*/

Product::Ptr
XMLFilesBackend::createProduct( const zypp::parser::yum::YUMProductData & parsed ) const
{
  try
  {
    shared_ptr<XMLProductImpl> impl(new XMLProductImpl());

    impl->_category = parsed.type;
    impl->_vendor = parsed.vendor;
    #warning "FIX when YUM parser uses TranslatedString"
    //impl->_displayname = parsed.displayname;
    //impl->_description = parsed.description;

    // Collect basic Resolvable data
    NVRAD dataCollect( parsed.name, Edition( parsed.ver, parsed.rel, parsed.epoch ), Arch_noarch, createDependencies(parsed, ResTraits<Product>::kind) );
    Product::Ptr product = detail::makeResolvableFromImpl( dataCollect, impl );
    return product;
  }
  catch (const Exception & excpt_r)
  {
    ERR << excpt_r << endl;
    throw "Cannot create product object";
  }
}

Selection::Ptr
XMLFilesBackend::createSelection( const zypp::parser::yum::YUMGroupData & parsed ) const
{
  try
  {
    DBG << parsed << std::endl;
    shared_ptr<XMLSelectionImpl> impl(new XMLSelectionImpl());
      /*
      YUMGroupData();
        std::string groupId;
        std::list<MultiLang> name;
        std::string default_;
        std::string userVisible;
        std::list<MultiLang> description;
        std::list<MetaPkg> grouplist;
        std::list<PackageReq> packageList;
      */
    impl->_summary = parsed.description;
    //impl->_description = parsed.description;
    impl->_name = parsed.groupId;
    //impl->_order = parsed.summary;
    //impl->_category = parsed.summary;
    impl->_visible = ((parsed.userVisible == "true") ? true : false);
    
    for( std::list<MetaPkg>::const_iterator it = parsed.grouplist.begin(); it != parsed.grouplist.end(); ++it)
    {
      DBG << "Selection dependencies" << std::endl;
      if ((*it).type == "optional" )
        impl->_suggests.insert((*it).name);
      if ((*it).type == "mandatory" )
        impl->_recommends.insert((*it).name);
    }
    for( std::list<PackageReq>::const_iterator it = parsed.packageList.begin(); it != parsed.packageList.end(); ++it)
    {
        DBG << "Selection package dependencies" << std::endl;
        impl->_install_packages.insert((*it).name);
    }
    // Collect basic Resolvable data
    NVRAD dataCollect( parsed.groupId, Edition::noedition, Arch_noarch, createGroupDependencies(parsed) );
    Selection::Ptr selection = detail::makeResolvableFromImpl( dataCollect, impl );
    return selection;
  }
  catch (const Exception & excpt_r)
  {
    ERR << excpt_r << endl;
    throw "Cannot create selection object";
  }
}

Dependencies 
XMLFilesBackend::createDependencies( const zypp::parser::yum::YUMObjectData & parsed, const Resolvable::Kind my_kind ) const
{
  Dependencies _deps;
  for (std::list<YUMDependency>::const_iterator it = parsed.provides.begin(); it != parsed.provides.end(); it++)
  {
    _deps[Dep::PROVIDES].insert(createCapability(*it, my_kind));
  }

  for (std::list<YUMDependency>::const_iterator it = parsed.conflicts.begin(); it != parsed.conflicts.end(); it++)
  {
    _deps[Dep::CONFLICTS].insert(createCapability(*it, my_kind));
  }
  
  for (std::list<YUMDependency>::const_iterator it = parsed.obsoletes.begin(); it != parsed.obsoletes.end(); it++)
  {
    _deps[Dep::OBSOLETES].insert(createCapability(*it, my_kind));
  }

  for (std::list<YUMDependency>::const_iterator it = parsed.freshen.begin(); it != parsed.freshen.end(); it++)
  {
    _deps[Dep::FRESHENS].insert(createCapability(*it, my_kind));
  }

  for (std::list<YUMDependency>::const_iterator it = parsed.requires.begin(); it != parsed.requires.end(); it++)
  {
    if (it->pre == "1")
      _deps[Dep::PREREQUIRES].insert(createCapability(*it, my_kind));
    else
      _deps[Dep::REQUIRES].insert(createCapability(*it, my_kind));
  }

    return _deps;
  }

Dependencies 
XMLFilesBackend::createGroupDependencies( const zypp::parser::yum::YUMGroupData & parsed ) const
{
  Dependencies _deps;

  for (std::list<PackageReq>::const_iterator it = parsed.packageList.begin(); it != parsed.packageList.end(); it++)
  {
    if (it->type == "mandatory" || it->type == "")
    {
      _deps[Dep::REQUIRES].insert(createCapability(YUMDependency( "", it->name, "EQ", it->epoch, it->ver, it->rel, "" ), ResTraits<Package>::kind));
    }
  }
  for (std::list<MetaPkg>::const_iterator it = parsed.grouplist.begin(); it != parsed.grouplist.end(); it++)
  {
    if (it->type == "mandatory" || it->type == "")
    {
      _deps[Dep::REQUIRES].insert(createCapability(YUMDependency("", it->name, "", "", "", "", "" ), ResTraits<Selection>::kind));
    }
  }
  return _deps;
}

Dependencies
XMLFilesBackend::createPatternDependencies( const zypp::parser::yum::YUMPatternData & parsed ) const
{
  Dependencies _deps;

  for (std::list<PackageReq>::const_iterator it = parsed.packageList.begin(); it != parsed.packageList.end(); it++)
  {
    if (it->type == "mandatory" || it->type == "")
    {
      _deps[Dep::REQUIRES].insert(createCapability(YUMDependency( "", it->name, "EQ", it->epoch, it->ver, it->rel, "" ), ResTraits<Package>::kind));
    }
  }
  for (std::list<MetaPkg>::const_iterator it = parsed.patternlist.begin(); it != parsed.patternlist.end(); it++)
  {
    if (it->type == "mandatory" || it->type == "")
    {
      _deps[Dep::REQUIRES].insert(createCapability(YUMDependency( "", it->name, "", "", "", "", "" ), ResTraits<Selection>::kind));
    }
  }
  return _deps;
}

Capability
XMLFilesBackend::createCapability(const YUMDependency & dep, const Resolvable::Kind & my_kind) const
{
  CapFactory _f;
  Resolvable::Kind _kind = dep.kind == "" ? my_kind : Resolvable::Kind(dep.kind);
  Capability cap;
  if ( ! dep.isEncoded() )
  {
    cap = _f.parse( _kind, dep.name, Rel(dep.flags), Edition(dep.ver, dep.rel, dep.epoch) );
  }
  else
  {
    cap = _f.parse( _kind, dep.encoded );
  }
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
  path source_p = path(d->root.asString()) / path(ZYPP_DB_DIR) / path ("source-cache");
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
    DBG << "[source-cache] - " << dir_itr->leaf() << std::endl;
    //sources.insert( sourceDataFromCacheFile( source_p + "/" + dir_itr->leaf() ) );
    std::string full_path = (source_p / dir_itr->leaf()).string();
    std::ifstream anIstream(full_path.c_str());
    XMLSourceCacheParser iter(anIstream, "");
    for (; ! iter.atEnd(); ++iter) {
      PersistentStorage::SourceData data = **iter;
      sources.push_back(data);
    }
  }
  MIL << "done reading source cache" << std::endl;
  return sources;

}

static std::string hexDigest(const md5_byte_t *digest)
{
  char s[33];
  int i;
  for (i=0; i<16; i++)
    sprintf(s+i*2, "%02x", digest[i]);

  s[32]='\0';
  return std::string(s);
}

void
XMLFilesBackend::storeSource(const PersistentStorage::SourceData &data)
{
  std::string xml = toXML(data);
  path source_p = path(d->root.asString()) / path(ZYPP_DB_DIR) / path ("source-cache");

  // generate a filename
  if (data.alias.size() == 0)
  {
    ZYPP_THROW(Exception("Cant save source with empty alias"));
  }
  //MD5 md5(ss);
  md5_state_t state;
  md5_byte_t digest[16];

  md5_init(&state);
  /* Append a string to the message. */
  std::string alias = data.alias;
  md5_append(&state, (md5_byte_t*) alias.c_str(), alias.size());  /* Finish the message and return the digest. */
  md5_finish(&state, digest);
  
  //DBG << std::endl << xml << std::endl;
  std::ofstream file;
  //DBG << filename << std::endl;
  try
  {
    std::string full_path = (source_p / hexDigest(digest)).string();
    
    file.open(full_path.c_str());
    file << xml;
    file.close();
  }
  catch ( std::exception &e )
  {
    ERR << "Error saving source " << data.alias << " in the cache" << std::endl;
    ZYPP_THROW(Exception(e.what()));
  }
}

void
XMLFilesBackend::deleteSource(const std::string &alias)
{
  
}

/////////////////////////////////////////////////////////////////
} // namespace storage
///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
