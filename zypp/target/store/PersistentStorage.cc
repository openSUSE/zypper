/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/target/store/PersistentStorage.cc
*
*/
#include <iostream>
#include "zypp/base/Logger.h"

#include "PersistentStorage.h"
#include "Backend.h"
//#include "XMLFilesBackend.h"

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
struct PersistentStorage::Private
{
  shared_ptr<Backend> backend;
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
: d( new Private )
{
  DBG << "  Creating XML Files backend" << endl;
}

void PersistentStorage::init(const Pathname &root)
{
  //d->backend.reset( new XMLFilesBackend(root) );
}

bool PersistentStorage::isInitialized() const
{
  return d->backend;
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
//   d->backend->doTest();
}

Date
PersistentStorage::timestamp() const
{
//   return d->backend->timestamp();
  return Date();
}

void
PersistentStorage::storeObject( ResObject::constPtr resolvable )
{
//   d->backend->storeObject(resolvable);
}

void
PersistentStorage::deleteObject( ResObject::constPtr resolvable )
{
//   d->backend->deleteObject(resolvable);
}

std::list<ResObject::Ptr>
PersistentStorage::storedObjects() const
{
//   return d->backend->storedObjects();
  return std::list<ResObject::Ptr>();
}

std::list<ResObject::Ptr>
PersistentStorage::storedObjects(const Resolvable::Kind kind) const
{
  //list<ResObject::Ptr>::iterator it;
  //it = find(nums.begin(), nums.end(), 3); // Search the list.
//   return d->backend->storedObjects(kind);
  return std::list<ResObject::Ptr>();
}

std::list<ResObject::Ptr>
PersistentStorage::storedObjects(const Resolvable::Kind kind, const std::string & name, bool partial_match) const
{
//   return d->backend->storedObjects(kind, name, partial_match);
  return std::list<ResObject::Ptr>();
}

/////////////////////////////////////////////////////////
// FLAGS API
////////////////////////////////////////////////////////

void
PersistentStorage::setObjectFlag( ResObject::constPtr resolvable, const std::string &flag )
{
//   d->backend->setObjectFlag(resolvable, flag);
}

void
PersistentStorage::removeObjectFlag( ResObject::constPtr resolvable, const std::string &flag )
{
//   d->backend->removeObjectFlag(resolvable, flag);
}

std::set<std::string>
PersistentStorage::objectFlags( ResObject::constPtr resolvable ) const
{
//   return d->backend->objectFlags(resolvable);
  return std::set<std::string>();
}

bool
PersistentStorage::doesObjectHasFlag( ResObject::constPtr resolvable, const std::string &flag ) const
{
//   return d->backend->doesObjectHasFlag(resolvable, flag);
  return false;
}

/////////////////////////////////////////////////////////
// Named Flags API
////////////////////////////////////////////////////////

void
PersistentStorage::setFlag( const std::string &key, const std::string &flag )
{
//   d->backend->setFlag(key, flag);
}

void
PersistentStorage::removeFlag( const std::string &key, const std::string &flag )
{
//   d->backend->removeFlag(key, flag);
}

std::set<std::string>
PersistentStorage::flags( const std::string &key ) const
{
//   return d->backend->flags(key);
  return std::set<std::string>();
}

bool
PersistentStorage::hasFlag( const std::string &key, const std::string &flag ) const
{
//   return d->backend->hasFlag(key, flag);
  return false;
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
