/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zmd/backend/dbsource/SqlitePatternImpl.h
 *
*/
#ifndef ZMD_BACKEND_DBSOURCE_DBPATTERNIMPL_H
#define ZMD_BACKEND_DBSOURCE_DBPATTERNIMPL_H

#include "zypp/detail/PatternImpl.h"
#include "zypp/Source.h"
#include <sqlite3.h>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//        CLASS NAME : SqlitePatternImpl
//
/** Class representing a package
*/
class SqlitePatternImpl : public detail::PatternImplIf
{
public:

	/** Default ctor
	*/
	SqlitePatternImpl( Source_Ref source_r );
	void readHandle( sqlite_int64 id, sqlite3_stmt *handle );

	/** Pattern summary */
	virtual TranslatedText summary() const;
	/** Pattern description */
	virtual TranslatedText description() const;

	virtual bool isDefault() const;

	virtual bool userVisible() const;

	virtual TranslatedText category() const;

	virtual Pathname icon() const;

	virtual Pathname script() const;
      
	virtual Label order() const;
	/** */
	virtual Source_Ref source() const;
        /** */
	virtual ZmdId zmdid() const;

protected:
	Source_Ref _source;
	TranslatedText _summary;
	TranslatedText _description;
	ZmdId _zmdid;
	bool _default;
	bool _visible;
	TranslatedText _category;
	Pathname _icon;
	Pathname _script;
	Label _order;
 };
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZMD_BACKEND_DBSOURCE_DBPATTERNIMPL_H
