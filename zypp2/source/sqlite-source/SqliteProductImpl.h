/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zmd/backend/dbsource/SqliteProductImpl.h
 *
*/
#ifndef ZMD_BACKEND_DBSOURCE_DBPRODUCTIMPL_H
#define ZMD_BACKEND_DBSOURCE_DBPRODUCTIMPL_H

#include "zypp/detail/ProductImpl.h"
#include "zypp/Source.h"
#include <sqlite3.h>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//        CLASS NAME : SqliteProductImpl
//
/** Class representing a package
*/
class SqliteProductImpl : public detail::ProductImplIf
{
public:

	/** Default ctor
	*/
	SqliteProductImpl( Source_Ref source_r );
	void readHandle( sqlite_int64 id, sqlite3_stmt *handle );

	/** Product summary */
	virtual TranslatedText summary() const;
	/** Product description */
	virtual TranslatedText description() const;
	/** Get the category of the product - addon or base*/
	virtual std::string category() const;
	/** Get the vendor of the product */
	virtual Label vendor() const;
	/** Get the name of the product to be presented to user */
	virtual Label displayName( const Locale & locale_r = Locale() ) const;
	/** */
	virtual Url releaseNotesUrl() const;
	/** */
	virtual Source_Ref source() const;
        /** */
	virtual ZmdId zmdid() const;

protected:
	TranslatedText _summary;
	TranslatedText _description;
	Source_Ref _source;
	ZmdId _zmdid;
	std::string _category;
	Label _vendor;
	Label _displayName;
	Url _releaseNotesUrl;
 };
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZMD_BACKEND_DBSOURCE_DBPRODUCTIMPL_H
