/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	devel/devel.dmacvicar/PersistentStorage.h
*
*/
#ifndef DEVEL_DEVEL_DMACVICAR_PERSISTENTSTORAGE_H
#define DEVEL_DEVEL_DMACVICAR_PERSISTENTSTORAGE_H

#include <iosfwd>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////
	namespace storage
	{ /////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////
		//
		//	CLASS NAME : PersistentStorage
		//
		/** */
		class PersistentStorage : public base::ReferenceCounted, private base::NonCopyable
		{
			friend std::ostream & operator<<( std::ostream & str, const PersistentStorage & obj );
			typedef intrusive_ptr<PersistentStorage> Ptr;
			typedef intrusive_ptr<const PersistentStorage> constPtr;
		public:
			/** Default ctor */
			PersistentStorage();
			/** Dtor */
			~PersistentStorage();
			void doTest();
		private:
			class Private;
			Private *d;
		};
		///////////////////////////////////////////////////////////////////
		/** \relates PersistentStorage Stream output */
		std::ostream & operator<<( std::ostream & str, const PersistentStorage & obj );

		/////////////////////////////////////////////////////////////////
	} // namespace devel.dmacvicar
	///////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////
} // namespace devel
///////////////////////////////////////////////////////////////////
#endif // DEVEL_DEVEL_DMACVICAR_PERSISTENTSTORAGE_H
