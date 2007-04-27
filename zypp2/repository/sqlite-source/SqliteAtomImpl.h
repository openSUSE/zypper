/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zmd/backend/dbsource/SqliteAtomImpl.h
 *
*/
#ifndef ZMD_BACKEND_DBSOURCE_DBATOMIMPL_H
#define ZMD_BACKEND_DBSOURCE_DBATOMIMPL_H

#include "zypp/detail/AtomImpl.h"
#include "zypp/Source.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//        CLASS NAME : SqliteAtomImpl
//
/** Class representing a package
*/
class SqliteAtomImpl : public detail::AtomImplIf
{
public:

	/** Default ctor
	*/
	SqliteAtomImpl( Source_Ref source_r, ZmdId zmdid );

	/** */
	virtual Source_Ref source() const;
        /** */
	virtual ZmdId zmdid() const;

protected:
	Source_Ref _source;
	ZmdId _zmdid;

 };
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZMD_BACKEND_DBSOURCE_DBATOMIMPL_H
