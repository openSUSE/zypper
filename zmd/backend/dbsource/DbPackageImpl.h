/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zmd/backend/dbsource/DbPackageImpl.h
 *
*/
#ifndef ZMD_BACKEND_DBSOURCE_DBPACKAGEIMPL_H
#define ZMD_BACKEND_DBSOURCE_DBPACKAGEIMPL_H

#include "zypp/detail/PackageImpl.h"
#include "zypp/Source.h"
#include <sqlite3.h>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//        CLASS NAME : DbPackageImpl
//
/** Class representing a package
*/
class DbPackageImpl : public detail::PackageImplIf
{
public:

	/** Default ctor
	*/
	DbPackageImpl( Source_Ref source_r );
	void readHandle( sqlite_int64 id, sqlite3_stmt *handle );

	/** Package summary */
	virtual TranslatedText summary() const;
	/** Package description */
	virtual TranslatedText description() const;
	virtual ByteCount size() const;
	/** */
	virtual PackageGroup group() const;
	/** */
	virtual ByteCount archivesize() const;
	/** */
	virtual Pathname location() const;
	/** */
	virtual bool installOnly() const;
	/** */
	virtual Source_Ref source() const;
        /** */
	virtual ZmdId zmdid() const;

protected:
	Source_Ref _source;
	TranslatedText _summary;
	TranslatedText _description;
	PackageGroup _group;
	Pathname _location;
	bool _install_only;
	ZmdId _zmdid;

	ByteCount _size_installed;
	ByteCount _size_archive;


 };
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZMD_BACKEND_DBSOURCE_DBPACKAGEIMPL_H
