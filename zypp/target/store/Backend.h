/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/target/store/Backend.h
*
*/
#ifndef ZYPP_STORAGE_BACKEND_H
#define ZYPP_STORAGE_BACKEND_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"
#include <zypp/Pathname.h>
#include <zypp/Patch.h>

#include <zypp/target/store/PersistentStorage.h>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace storage
{ /////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : Backend
//
/**
* This class represents a storage backend implementation
*/
class Backend
{
  friend std::ostream & operator<<( std::ostream & str, const Backend & obj );
public:
  /** root is the system root path */
  Backend(const Pathname &root);
  /** Dtor */
  virtual ~Backend();
  virtual void doTest() = 0;

  /**
  * is the storage backend initialized
  */
  virtual bool isBackendInitialized() const = 0;
  /**
  * initialize the storage backend
  */
  virtual void initBackend() = 0;

  /**
   * timestamp of last modification
   */
  virtual Date timestamp() const = 0;
  
  /**
  * Stores a Resolvable in the active backend.
  */
  virtual void storeObject( ResObject::constPtr resolvable )  = 0;
  /**
  * Deletes a Resolvable from the active backend.
  */
  virtual void deleteObject( ResObject::constPtr resolvable ) = 0;

  /**
  * Query for installed Resolvables.
  */
  virtual std::list<ResObject::Ptr> storedObjects() const = 0;
  /**
  * Query for installed Resolvables of a certain kind
  */
  virtual std::list<ResObject::Ptr> storedObjects(const Resolvable::Kind) const = 0;
  /**
  * Query for installed Resolvables of a certain kind by name
  * \a partial_match allows for text search.
  */
  virtual std::list<ResObject::Ptr> storedObjects(const Resolvable::Kind, const std::string & name, bool partial_match = false) const = 0;

  /////////////////////////////////////////////////////////
  // Resolvables Flags API
  ////////////////////////////////////////////////////////
  public:
  virtual void setObjectFlag( ResObject::constPtr resolvable, const std::string &flag ) = 0;
  virtual void removeObjectFlag( ResObject::constPtr resolvable, const std::string &flag ) = 0;
  virtual std::set<std::string> objectFlags( ResObject::constPtr resolvable ) const = 0;
  virtual bool doesObjectHasFlag( ResObject::constPtr resolvable, const std::string &flag ) const = 0;

  /////////////////////////////////////////////////////////
  // Named Flags API
  ////////////////////////////////////////////////////////
  public:
  virtual void setFlag( const std::string &key, const std::string &flag ) = 0;
  virtual void removeFlag( const std::string &key, const std::string &flag ) = 0;
  virtual std::set<std::string> flags( const std::string &key ) const = 0;
  virtual bool hasFlag( const std::string &key, const std::string &flag ) const = 0;

  /////////////////////////////////////////////////////////
  // SOURCES API
  ////////////////////////////////////////////////////////
  /**
    * Query for installed Sources
    */
  virtual std::list<PersistentStorage::SourceData> storedSources() const = 0;
  /**
    * Query for installed Sources
    */
  virtual void storeSource(const PersistentStorage::SourceData &data) = 0;
  /**
    * Query for installed Sources
    */
  virtual void deleteSource(const std::string &alias) = 0;

private:
  /** Pointer to implementation */
  class Private;
  Private *d;
};
///////////////////////////////////////////////////////////////////

/** \relates Backend Stream output */
std::ostream & operator<<( std::ostream & str, const Backend & obj );

/////////////////////////////////////////////////////////////////
} // namespace storage
///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // DEVEL_DEVEL_DMACVICAR_BACKEND_H
