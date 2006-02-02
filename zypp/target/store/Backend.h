/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	devel/devel.dmacvicar/Backend.h
*
*/
#ifndef ZYPP_STORAGE_BACKEND_H
#define ZYPP_STORAGE_BACKEND_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"
#include <zypp/Patch.h>

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
  /** Default ctor */
  Backend();
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
  * Stores a Resolvable in the active backend.
  */
  virtual void storeObject( Resolvable::constPtr resolvable )  = 0;
  /**
  * Deletes a Resolvable from the active backend.
  */
  virtual void deleteObject( Resolvable::Ptr resolvable ) = 0;

  /**
  * Query for installed Resolvables.
  */
  virtual std::list<Resolvable::Ptr> storedObjects() const = 0;
  /**
  * Query for installed Resolvables of a certain kind
  */
  virtual std::list<Resolvable::Ptr> storedObjects(const Resolvable::Kind) const = 0;
  /**
  * Query for installed Resolvables of a certain kind by name
  * \a partial_match allows for text search.
  */
  virtual std::list<Resolvable::Ptr> storedObjects(const Resolvable::Kind, const std::string & name, bool partial_match = false) const = 0;

private:
  /** Pointer to implementation */
  class Private;
  Private *d;
};
///////////////////////////////////////////////////////////////////

/** \relates Backend Stream output */
std::ostream & operator<<( std::ostream & str, const Backend & obj );

/////////////////////////////////////////////////////////////////
} // namespace devel.dmacvicar
///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
} // namespace devel
///////////////////////////////////////////////////////////////////
#endif // DEVEL_DEVEL_DMACVICAR_BACKEND_H
