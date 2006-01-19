/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */

#include "zypp/target/rpm/librpmDb.h"
#include "zypp/base/Logger.h"

using namespace zypp::target::rpm;

using std::endl;

#include <sqlite3.h>
#include "package-writer.h"

int
main (int argc, char **argv)
{
    if (argc != 2) {
	ERR << "usage: " << argv[0] << " <database>" << endl;
	return 1;
    }

    librpmDb::unblockAccess();
    librpmDb::dbAccess (librpmDb::defaultRoot(), librpmDb::defaultDbPath());

    librpmDb::constPtr rpmhandle;
    librpmDb::dbAccess (rpmhandle);

    write_packages_to_db (argv[1]);

    return 0;
}
