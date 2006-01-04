/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	devel/devel.dmacvicar/BDBBackend.cc
*
*/
#include <iostream>
//#include "zypp/base/Logger.h"

#include "BDBBackend.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////
	namespace storage
	{
		//
		//	CLASS NAME : BDBBackend::Private
		//
		///////////////////////////////////////////////////////////////////

		class BDBBackend::Private
		{
			
		};

		///////////////////////////////////////////////////////////////////
		//
		//	CLASS NAME : BDBBackend
		//
		///////////////////////////////////////////////////////////////////

		///////////////////////////////////////////////////////////////////
		//
		//	METHOD NAME : BDBBackend::BDBBackend
		//	METHOD TYPE : Ctor
		//
		BDBBackend::BDBBackend()
		{}

		///////////////////////////////////////////////////////////////////
		//
		//	METHOD NAME : BDBBackend::~BDBBackend
		//	METHOD TYPE : Dtor
		//
		BDBBackend::~BDBBackend()
		{}

		///////////////////////////////////////////////////////////////////
		//
		//	METHOD NAME : BDBBackend::doTest()
		//	METHOD TYPE : Dtor
		//
		void BDBBackend::doTest()
		{}


			bool BDBBackend::isDatabaseInitialized()
			{
				return true;
			}

			void BDBBackend::initDatabaseForFirstTime()
			{
				
			}

		/******************************************************************
		**
		**	FUNCTION NAME : operator<<
		**	FUNCTION TYPE : std::ostream &
		*/
		std::ostream & operator<<( std::ostream & str, const BDBBackend & obj )
		{
			return str;
		}

		/////////////////////////////////////////////////////////////////
	} // namespace devel.dmacvicar
	///////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////
} // namespace devel
///////////////////////////////////////////////////////////////////
