#include "zypp/target/rpm/librpmDb.h"
#include "zypp/base/Logger.h"

using namespace zypp::target::rpm;

using std::endl;

int
main (int argc, char *argv[])
{
    librpmDb::unblockAccess();
    librpmDb::dbAccess (librpmDb::defaultRoot(), librpmDb::defaultDbPath());

    librpmDb::constPtr rpmhandle;
    librpmDb::dbAccess (rpmhandle);

    librpmDb::db_const_iterator iter;

    if (iter.dbHdrNum() == 0)
	return 1;

    int count = 0;
    while (*iter) {
	DBG << *(*iter) << endl;
	++count;
	++iter;
    }
    printf ("%d packages\n", count);
    return 0;
}
