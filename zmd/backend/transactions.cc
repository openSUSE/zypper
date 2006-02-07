/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */

/** file: zmd/backend/transactions.cc
    read/write 'transactions' table to/from ResPool
 */

#include "transactions.h"

#include "zypp/ZYpp.h"
#include "zypp/ZYppFactory.h"

#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/base/Algorithm.h"

#include "zypp/ResPool.h"
#include "zypp/ResFilters.h"
#include "zypp/CapFilters.h"

using std::endl;
using namespace zypp;

#include <sqlite3.h>
#include "dbsource/DbAccess.h"
#include "dbsource/DbSources.h"

typedef enum {
    PACKAGE_OP_REMOVE  = 0,
    PACKAGE_OP_INSTALL = 1,
    PACKAGE_OP_UPGRADE = 2		// unused
} PackageOpType;

using namespace std;
using namespace zypp;

//-----------------------------------------------------------------------------

bool
read_transactions (const ResPool & pool, sqlite3 *db, const DbSources & sources)
{
    MIL << "read_transactions" << endl;

    sqlite3_stmt *handle = NULL;
    const char  *sql = "SELECT action, id FROM transactions";
    int rc = sqlite3_prepare (db, sql, -1, &handle, NULL);
    if (rc != SQLITE_OK) {
	ERR << "Can not prepare transaction insertion clause: " << sqlite3_errmsg (db) << endl;
        return false;
    }

    while ((rc = sqlite3_step (handle)) == SQLITE_ROW) {
        int id;
        PackageOpType action;
	ResObject::constPtr obj;

        action = (PackageOpType) sqlite3_column_int (handle, 0);
        id = sqlite3_column_int (handle, 1);

	obj = sources.getById (id);
	if (obj == NULL) {
	    ERR << "Can't find resolvable id " << id << endl;
	    break;
	}

	ResPool::const_nameiterator pend = pool.nameend(obj->name());
	for (ResPool::const_nameiterator it = pool.namebegin(obj->name()); it != pend; ++it) {
	    PoolItem pos = it->second;
	    if (pos.resolvable() == obj)
	    {
		MIL << "Found item " << id << ": " << pos << ", action " << action << endl;
		switch (action) {
		    case PACKAGE_OP_REMOVE:
			pos.status().setToBeUninstalled(ResStatus::USER);
			break;
		    case PACKAGE_OP_INSTALL:
			pos.status().setToBeInstalled(ResStatus::USER);
			break;
		    case PACKAGE_OP_UPGRADE:
			pos.status().setToBeInstalled(ResStatus::USER);
			break;
		    default:
			ERR << "Ignoring unknown action " << action << endl;
			break;
		}
		break;
	    }
	}
    }

    sqlite3_finalize (handle);

    if (rc != SQLITE_DONE) {
	ERR << "Error reading transaction packages: " << sqlite3_errmsg (db) << endl;
	return false;
    }

    return true;
}



