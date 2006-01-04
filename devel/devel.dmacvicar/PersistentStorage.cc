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

#include "devel/devel.dmacvicar/PersistentStorage.h"
#include "devel/devel.dmacvicar/Backend.h"
#include "devel/devel.dmacvicar/SQLiteBackend.h"
#include "devel/devel.dmacvicar/BDBBackend.h"

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
			DBG << "  Creating SQLite backend" << endl;
			d->backend = new SQLiteBackend();
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
	} // namespace devel.dmacvicar
	///////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////
} // namespace devel
///////////////////////////////////////////////////////////////////
