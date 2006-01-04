/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	devel/devel.dmacvicar/SQLiteBackend.h
*
*/
#ifndef DEVEL_DEVEL_DMACVICAR_SQLITEBACKEND_H
#define DEVEL_DEVEL_DMACVICAR_SQLITEBACKEND_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"
#include "devel/devel.dmacvicar/Backend.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////
	namespace storage
	{ /////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////
	//
	//	CLASS NAME : SQLiteBackend
	//
	/** */
	class SQLiteBackend : public Backend
	{
		friend std::ostream & operator<<( std::ostream & str, const SQLiteBackend & obj );
	public:
	/** Implementation  */
		class Impl;
	public:
		/** Default ctor */
		SQLiteBackend();
		/** Dtor */
		~SQLiteBackend();
		void doTest();
		bool isDatabaseInitialized();
		void initDatabaseForFirstTime();

		void insertTest();
	private:
		class Private;
		Private *d;
	};
		///////////////////////////////////////////////////////////////////
	/** \relates SQLiteBackend Stream output */
	std::ostream & operator<<( std::ostream & str, const SQLiteBackend & obj );
		/////////////////////////////////////////////////////////////////
	} // namespace devel.dmacvicar
	///////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////
} // namespace devel
///////////////////////////////////////////////////////////////////
#endif // DEVEL_DEVEL_DMACVICAR_SQLITEBACKEND_H
