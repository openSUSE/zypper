/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	devel/devel.dmacvicar/BDBBackend.h
*
*/
#ifndef DEVEL_DEVEL_DMACVICAR_BDBBACKEND_H
#define DEVEL_DEVEL_DMACVICAR_BDBBACKEND_H

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
		//	CLASS NAME : BDBBackend
		//
		/** */
		class BDBBackend : public Backend
		{
			friend std::ostream & operator<<( std::ostream & str, const BDBBackend & obj );

		public:
			/** Default ctor */
			BDBBackend();
			/** Dtor */
			~BDBBackend();
			void doTest();
			bool isDatabaseInitialized();
			void initDatabaseForFirstTime();
		private:
			/** Pointer to implementation */
			class Private;
			Private *d;
		};
		///////////////////////////////////////////////////////////////////

		/** \relates BDBBackend Stream output */
		std::ostream & operator<<( std::ostream & str, const BDBBackend & obj );

		/////////////////////////////////////////////////////////////////
	} // namespace devel.dmacvicar
	///////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////
} // namespace devel
///////////////////////////////////////////////////////////////////
#endif // DEVEL_DEVEL_DMACVICAR_BDBBACKEND_H
