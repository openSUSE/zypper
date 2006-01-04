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
#ifndef DEVEL_DEVEL_DMACVICAR_BACKEND_H
#define DEVEL_DEVEL_DMACVICAR_BACKEND_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"

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
		/** */
		class Backend
		{
			friend std::ostream & operator<<( std::ostream & str, const Backend & obj );
		public:
			/** Default ctor */
			Backend();
			/** Dtor */
			virtual ~Backend();
			virtual void doTest() = 0;
			virtual bool isDatabaseInitialized() = 0;
			virtual void initDatabaseForFirstTime() = 0;
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
