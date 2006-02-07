/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* DbSources.h: sqlite catalogs table reader
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

#ifndef ZMD_BACKEND_DBSOURCES_H
#define ZMD_BACKEND_DBSOURCES_H

#include <iosfwd>
#include <string>
#include <list>
#include <map>

#include <sqlite3.h>
#include "zypp/Source.h"
#include "zypp/PoolItem.h"

#include "DbAccess.h"

///////////////////////////////////////////////////////////////////
//
//      CLASS NAME : DbSources

typedef std::list<zypp::Source_Ref> SourcesList;

class DbSources
{
  private:
     sqlite3 *_db;
     SourcesList _sources;
     IdMap _idmap;

  public:

    DbSources (sqlite3 *db);
    virtual ~DbSources();

    const SourcesList & sources (bool refresh = false);
    zypp::ResObject::constPtr getById (sqlite_int64 id) const;
};

#endif  // ZMD_BACKEND_DBSOURCES_H
