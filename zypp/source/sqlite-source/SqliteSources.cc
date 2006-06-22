/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* SqliteSources.cc  wrapper for zmd.db 'catalogs' table
 *
 * Copyright (C) 2000-2002 Ximian, Inc.
 * Copyright (C) 2005 SUSE Linux Products GmbH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"

#include "zypp/Source.h"
#include "zypp/SourceFactory.h"
#include "zypp/SourceManager.h"
#include "zypp/source/SourceImpl.h"

#include "zypp/media/MediaManager.h"

#include "SqliteSources.h"
#include "SqliteSourceImpl.h"
#include <sqlite3.h>

using namespace std;
using namespace zypp;

//---------------------------------------------------------------------------

SqliteSources::SqliteSources (sqlite3 *db)
    : _db (db)
{ 
    MIL << "SqliteSources::SqliteSources(" << db << ")" << endl;
}

SqliteSources::~SqliteSources ()
{
}


ResObject::constPtr
SqliteSources::getById (sqlite_int64 id) const
{
    IdMap::const_iterator it = _idmap.find(id);
    if (it == _idmap.end())
	return NULL;
    return it->second;
}


Source_Ref
SqliteSources::createDummy( const Url & url, const string & catalog )
{
    media::MediaManager mmgr;
    media::MediaId mediaid = mmgr.open( url );
    SourceFactory factory;

    try {

	SqliteSourceImpl *impl = new SqliteSourceImpl ();
	impl->factoryCtor( mediaid, Pathname(), catalog );
	impl->setId( catalog );
	impl->setZmdName( catalog );
	impl->setZmdDescription ( catalog );
	impl->setPriority( 0 );
	impl->setSubscribed( true );

	Source_Ref src( factory.createFrom( impl ) );
	return src;
    }
    catch (Exception & excpt_r) {
	ZYPP_CAUGHT(excpt_r);
    }

    return Source_Ref();
}


//
// actually create sources from catalogs table
// if zypp_restore == true, match catalogs entries with actual zypp sources
//   and return zypp source on match
// else create dummy file:/ source since a real source is either not needed
//   or files are provided by zmd
//

const SourcesList &
SqliteSources::sources( bool zypp_restore, bool refresh )
{
    MIL << "SqliteSources::sources(" << (zypp_restore ? "zypp_restore " : "") << (refresh ? "refresh" : "") << ")" << endl;

    if (_db == NULL)
	return _sources;

    if (!refresh
	&& !_sources.empty())
    {
	return _sources;
    }

    _sources.clear();

    const char *query =
	//      0   1     2      3            4         5
	"SELECT id, name, alias, description, priority, subscribed "
	"FROM catalogs";

    sqlite3_stmt *handle = NULL;
    int rc = sqlite3_prepare (_db, query, -1, &handle, NULL);
    if (rc != SQLITE_OK) {
	ERR << "Can not read 'channels': " << sqlite3_errmsg (_db) << endl;
	return _sources;
    }

    media::MediaManager mmgr;
    _smgr = SourceManager::sourceManager();

    try {
	_smgr->restore("/");
    }
    catch (Exception & excpt_r) {
	ZYPP_CAUGHT (excpt_r);
	ERR << "Couldn't restore sources" << endl;
	return _sources;
    }

    media::MediaId mediaid = mmgr.open( Url( "file:/" ) );
    SourceFactory factory;

    // read catalogs table

    while ((rc = sqlite3_step (handle)) == SQLITE_ROW) {
	const char *text = (const char *) sqlite3_column_text( handle, 0 );
	if (text == NULL) {
	    ERR << "Catalog id is NULL" << endl;
	    continue;
	}
	string id (text);
	string name;
	text = (const char *) sqlite3_column_text( handle, 1 );
	if (text != NULL) name = text;
	string alias;
	text = (const char *) sqlite3_column_text( handle, 2 );
	if (text != NULL) alias = text;
	string desc;
	text = (const char *) sqlite3_column_text( handle, 3 );
	if (text != NULL) desc = text;
	unsigned priority = sqlite3_column_int( handle, 4 );
	int subscribed = sqlite3_column_int( handle, 5 );

	MIL << "id " << id
	    << ", name " << name
	    << ", alias " << alias
	    << ", desc " << desc
	    << ", prio " << priority
	    << ", subs " << subscribed
	    << endl;

	if (alias.empty()) alias = name;
	if (desc.empty()) desc = alias;

	Source_Ref zypp_source;

	if (zypp_restore
	    && id[0] != '@')		// not for zmd '@system' and '@local'
	{
	    MIL << "Try to find '" << name << "' as zypp source" << endl;
	    try {
		zypp_source = _smgr->findSource( name );
	    }
	    catch( const Exception & excpt_r ) {
		ZYPP_CAUGHT(excpt_r);

		MIL << "Try to find '" << alias << "' as zypp source" << endl;
		try {
		    zypp_source = _smgr->findSource( alias );
		}
		catch( const Exception & excpt_r ) {
		    ZYPP_CAUGHT(excpt_r);

		    // #177543
		    MIL << "Try to find URL '" << id << "' as zypp source" << endl;
		    try {
			Url url = id;
			zypp_source = _smgr->findSourceByUrl( url );
		    }
		    catch( const Exception & excpt_r ) {
			ZYPP_CAUGHT(excpt_r);
		    }
		}
	    }

	    if (zypp_source) {
		zypp_source.setId( id );		// set id, to match resolvable catalog
		MIL << "Found " << zypp_source << endl;
	    }
	}

	try {

	    SqliteSourceImpl *impl = new SqliteSourceImpl ();
	    impl->factoryCtor( mediaid, Pathname(), alias );
	    impl->setId( id );
	    impl->setZmdName( name );
	    impl->setZmdDescription ( desc );
	    impl->setPriority( priority );
	    impl->setSubscribed( subscribed != 0 );

	    impl->attachDatabase( _db );
	    impl->attachIdMap( &_idmap );
	    impl->attachZyppSource( zypp_source );

	    Source_Ref src( factory.createFrom( impl ) );
	    _sources.push_back( src );
	}
	catch (Exception & excpt_r) {
	    ZYPP_CAUGHT(excpt_r);
	    ERR << "Couldn't create zmd source" << endl;
	}

    }

    if (rc != SQLITE_DONE) {
	ERR << "Error while reading 'channels': " << sqlite3_errmsg (_db) << endl;
	_sources.clear();
    }

    MIL << "Read " << _sources.size() << " catalogs" << endl;
    return _sources;
}
