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

#include "zypp/base/PtrTypes.h"
#include "Backend.h"

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
  XMLFilesBackend();
  /** Dtor */
  ~XMLFilesBackend();
  virtual void doTest();

  /**
    * is the storage backend initialized
    */
  virtual bool isBackendInitialized();
  /**
    * initialize the storage backend
    */
  virtual void initBackend();

  /**
    * Stores a Resolvable in the active backend.
    */
  virtual void storeObject( Resolvable::Ptr resolvable ) ;
  /**
    * Deletes a Resolvable from the active backend.
    */
  virtual void deleteObject( Resolvable::Ptr resolvable );
  /**
    * Deletes a Resolvable from the active backend.
    */
  virtual std::list<Resolvable::Ptr> storedObjects();
   /**
    * Query for installed Resolvables of a certain kind
    */
  virtual std::list<Resolvable::Ptr> storedObjects(const Resolvable::Kind);
  /**
    * Query for installed Resolvables of a certain kind by name
    * \a partial_match allows for text search.
    */
  virtual std::list<Resolvable::Ptr> storedObjects(const Resolvable::Kind, const std::string & name, bool partial_match = false);

  protected:
  std::string randomFileName() const;
  
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
