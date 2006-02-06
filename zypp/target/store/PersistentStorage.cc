/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	devel/devel.dmacvicar/PersistentStorage.cc
*
*/
#include <iostream>
#include "zypp/base/Logger.h"

#include "PersistentStorage.h"
#include "Backend.h"
#include "XMLFilesBackend.h"
//#include "devel/devel.dmacvicar/BDBBackend.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace storage
{
///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : PersistentStoragePrivate
//
///////////////////////////////////////////////////////////////////
class PersistentStorage::Private
{
  public:
  Backend *backend;
};

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : PersistentStorage
//
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : PersistentStorage::PersistentStorage
//	METHOD TYPE : Ctor
//
PersistentStorage::PersistentStorage()
{
  d = new Private;
  DBG << "  Creating XML Files backend" << endl;
  d->backend = 0L;
}

void PersistentStorage::init(const Pathname &root)
{
  d->backend = new XMLFilesBackend(root);
}

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : PersistentStorage::~PersistentStorage
//	METHOD TYPE : Dtor
//
PersistentStorage::~PersistentStorage()
{}

    ///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : PersistentStorage::~PersistentStorage
//	METHOD TYPE : Dtor
//
void PersistentStorage::doTest()
{
  d->backend->doTest();
}

void
PersistentStorage::storeObject( Resolvable::constPtr resolvable )
{
  d->backend->storeObject(resolvable);
}

void
PersistentStorage::deleteObject( Resolvable::Ptr resolvable )
{

}

std::list<Resolvable::Ptr>
PersistentStorage::storedObjects() const
{
  return d->backend->storedObjects();
}

std::list<Resolvable::Ptr>
PersistentStorage::storedObjects(const Resolvable::Kind kind) const
{
  //list<Resolvable::Ptr>::iterator it;
  //it = find(nums.begin(), nums.end(), 3); // Search the list.
  return d->backend->storedObjects(kind);
}

std::list<Resolvable::Ptr>
PersistentStorage::storedObjects(const Resolvable::Kind kind, const std::string & name, bool partial_match) const
{
  return d->backend->storedObjects(kind, name, partial_match);
}

/////////////////////////////////////////////////////////
// SOURCES API
////////////////////////////////////////////////////////

std::list<PersistentStorage::SourceData>
PersistentStorage::storedSources() const
{
  return d->backend->storedSources();
}

void
PersistentStorage::storeSource(const PersistentStorage::SourceData &data)
{
  d->backend->storeSource(data);
}

void
PersistentStorage::deleteSource(const std::string &alias)
{
  d->backend->deleteSource(alias);
}

/******************************************************************
**
**	FUNCTION NAME : operator<<
**	FUNCTION TYPE : std::ostream &
*/
std::ostream & operator<<( std::ostream & str, const PersistentStorage & obj )
{
  //return str << *obj._pimpl;
  return str;
}
  /////////////////////////////////////////////////////////////////
} // namespace storage
///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
