//
// sources.cc
//
// read catalogs table and populate sources with resolvables
//

#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/ResStore.h"
#include "zmd/backend/dbsource/DbSources.h"
#include "zmd/backend/dbsource/DbAccess.h"


using std::endl;

int
main(int argc, char *argv[])
{
    if (argc != 2) {
	ERR << "usage: " << argv[0] << " <database>" << endl;
	return 1;
    }

    MIL << "Creating db" << endl;

    DbAccess db (argv[1]);

    if (!db.openDb(false))
	return 1;

    MIL << "Calling dbsources" << endl;

    DbSources s(db.db());

    const SourcesList & sources = s.sources();

    for (SourcesList::const_iterator it = sources.begin(); it != sources.end(); ++it) {
	zypp::ResStore store = it->resolvables();
	MIL << "Source '" << it->alias() << "' provides " << store.size() << " resolvables" << endl;
	int num = 0;
	for (zypp::ResStore::const_iterator it = store.begin(); it != store.end(); ++it)
	    MIL << ++num << ": " << **it << endl;
    }

    return 0;
}
