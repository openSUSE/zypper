#include "zypp/Pathname.h"
#include "zypp2/cache/sqlite3x/sqlite3x.hpp"
#include "zypp/base/PtrTypes.h"

#ifndef CACHE_SQLITE_H
#define CACHE_SQLITE_H

namespace sqlite3x {

typedef zypp::shared_ptr<sqlite3_command> sqlite3_command_ptr;
typedef zypp::shared_ptr<sqlite3_reader> sqlite3_reader_ptr;

}

namespace zypp { namespace cache {

struct DatabaseContext
{
  sqlite3x::sqlite3_connection con;
  sqlite3x::sqlite3_command_ptr select_versionedcap_cmd;
  sqlite3x::sqlite3_command_ptr select_namedcap_cmd;
  sqlite3x::sqlite3_command_ptr select_filecap_cmd;
  Pathname dbdir;
};

typedef zypp::shared_ptr<DatabaseContext> DatabaseContext_Ptr;

} }

#endif

