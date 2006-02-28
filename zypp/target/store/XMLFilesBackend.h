/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	devel/devel.dmacvicar/XMLFilesBackend.h
*
*/
#ifndef DEVEL_DEVEL_DMACVICAR_XMLFILESBACKEND_H
#define DEVEL_DEVEL_DMACVICAR_XMLFILESBACKEND_H

#include <iosfwd>


#include "zypp/Message.h"
#include "zypp/Patch.h"
#include "zypp/Script.h"
#include "zypp/Product.h"
#include "zypp/Selection.h"
#include "zypp/Pattern.h"

#include "zypp/parser/yum/YUMParser.h"
#include "zypp/parser/yum/YUMParserData.h"
#include "zypp/base/PtrTypes.h"
#include "Backend.h"

using namespace zypp;
using namespace zypp::parser::yum;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace storage
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : XMLFilesBackend
//
/** */
class XMLFilesBackend : public Backend
{
  friend std::ostream & operator<<( std::ostream & str, const XMLFilesBackend & obj );
public:
  typedef intrusive_ptr<XMLFilesBackend> Ptr;
  typedef intrusive_ptr<const XMLFilesBackend> constPtr;

  public:
  /** Default ctor */
  XMLFilesBackend(const Pathname &root);
  /** Dtor */
  ~XMLFilesBackend();
  void setRandomFileNameEnabled( bool enabled );
  virtual void doTest();

  /**
    * is the storage backend initialized
    */
  virtual bool isBackendInitialized() const;
  /**
    * initialize the storage backend
    */
  virtual void initBackend();
  /**
    * Stores a Resolvable in the active backend.
    */
  virtual void storeObject( ResObject::constPtr resolvable ) ;
  /**
    * Deletes a Resolvable from the active backend.
    */
  virtual void deleteObject( ResObject::constPtr resolvable );
  /**
    * Deletes a Resolvable from the active backend.
    */
  virtual std::list<ResObject::Ptr> storedObjects() const;
   /**
    * Query for installed Resolvables of a certain kind
    */
  virtual std::list<ResObject::Ptr> storedObjects(const Resolvable::Kind) const;
  /**
    * Query for installed Resolvables of a certain kind by name
    * \a partial_match allows for text search.
    */
  virtual std::list<ResObject::Ptr> storedObjects(const Resolvable::Kind, const std::string & name, bool partial_match = false) const;

  /////////////////////////////////////////////////////////
  // Resolvables Flags API
  ////////////////////////////////////////////////////////
  public:
  void setObjectFlag( ResObject::constPtr resolvable, const std::string &flag );
  void removeObjectFlag( ResObject::constPtr resolvable, const std::string &flag );
  std::set<std::string> objectFlags( ResObject::constPtr resolvable );
  
  protected:
  void writeObjectFlags( ResObject::constPtr resolvable, const std::set<std::string> &flags );
  /////////////////////////////////////////////////////////
  // SOURCES API
  ////////////////////////////////////////////////////////
  public:
  /**
    * Query for installed Sources
    */
  virtual std::list<PersistentStorage::SourceData> storedSources() const;
  /**
    * Query for installed Sources
    */
  virtual void storeSource(const PersistentStorage::SourceData &data);
  /**
    * Query for installed Sources
    */
  virtual void deleteSource(const std::string &alias);
  

  protected:
  std::string randomString(int length) const;
  int random() const;

  
  /**
    * Directory where the xml file is stored (for the given resolvable)
    */
  std::string dirForResolvable( ResObject::constPtr resolvable ) const;
   /**
    * Directory where the flags are stored (for the given resolvable)
    */
  std::string dirForResolvableFlags( ResObject::constPtr resolvable ) const;
  /**
    * Encoded filename for a resolvable. Does not take kind into account.
    */
  std::string fileNameForResolvable( ResObject::constPtr resolvable ) const;
  /**
    * Encoded filename for resolvable flags. Does not take kind into account.
    */  
  std::string fullPathForResolvableFlags( ResObject::constPtr resolvable ) const;

  /**
    * Directory where the xml file is stored (for the given resolvable kind)
    */
  std::string dirForResolvableKind( Resolvable::Kind kind ) const;
  /**
    * Directory where the flags are stored (for the given resolvable kind)
    */
  std::string dirForResolvableKindFlags( Resolvable::Kind kind ) const;
  /**
    * Full path to the xml file for a given resolvable
    * Does not care if the resolvable is yet stored or not
    */
  std::string fullPathForResolvable( ResObject::constPtr resolvable ) const;
   /**
    * Full path to the xml file for a given resolvable
    * Does not care if the resolvable is yet stored or not
    */
  ResObject::Ptr resolvableFromFile( std::string file_path, Resolvable::Kind kind ) const;

  Patch::Ptr createPatch( const zypp::parser::yum::YUMPatchData & parsed ) const;
  Message::Ptr createMessage( const zypp::parser::yum::YUMPatchMessage & parsed ) const;
  Script::Ptr createScript(const zypp::parser::yum::YUMPatchScript & parsed ) const;
  Product::Ptr createProduct( const zypp::parser::yum::YUMProductData & parsed ) const;
  Selection::Ptr createSelection( const zypp::parser::yum::YUMPatternData & parsed ) const;
  Pattern::Ptr createPattern( const zypp::parser::yum::YUMPatternData & parsed ) const;

  Dependencies createDependencies( const zypp::parser::yum::YUMObjectData & parsed, const Resolvable::Kind my_kind ) const;
  Dependencies createGroupDependencies( const zypp::parser::yum::YUMGroupData & parsed ) const;
  Dependencies createPatternDependencies( const zypp::parser::yum::YUMPatternData & parsed ) const;
  Capability createCapability(const YUMDependency & dep, const Resolvable::Kind & my_kind) const;

  private:
  class Private;
  Private *d;
};

///////////////////////////////////////////////////////////////////
/** \relates XMLFilesBackend Stream output */
std::ostream & operator<<( std::ostream & str, const XMLFilesBackend & obj );
/////////////////////////////////////////////////////////////////
} // namespace devel.dmacvicar
///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
} // namespace devel
///////////////////////////////////////////////////////////////////
#endif // DEVEL_DEVEL_DMACVICAR_SQLITEBACKEND_H
