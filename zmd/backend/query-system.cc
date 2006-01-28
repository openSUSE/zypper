/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */

#include "zypp/ZYpp.h"
#include "zypp/ZYppFactory.h"
#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"

using std::endl;
using namespace zypp;

#include <sqlite3.h>
#include "package-writer.h"

int
main (int argc, char **argv)
{
    if (argc != 2) {
	ERR << "usage: " << argv[0] << " <database>" << endl;
	return 1;
    }

    ZYppFactory zf;
    ZYpp::Ptr God = zf.letsTest();

    try {
	God->initTarget("/");
    }
    catch (Exception & excpt_r) {
	ZYPP_CAUGHT (excpt_r);
	return 1;
    }

    write_packages_to_db (argv[1], God->target()->resolvables());

    return 0;
}
