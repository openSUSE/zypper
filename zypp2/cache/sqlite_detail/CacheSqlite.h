#include "zypp/Pathname.h"
#include "zypp2/cache/sqlite3x/sqlite3x.hpp"
#include "zypp/base/PtrTypes.h"

#ifndef CACHE_SQLITE_H
#define CACHE_SQLITE_H

namespace sqlite3x {

typedef zypp::shared_ptr<sqlite3_command> sqlite3_command_ptr;
typedef zypp::shared_ptr<sqlite3_reader> sqlite3_reader_ptr;
typedef zypp::shared_ptr<sqlite3_connection> sqlite3_connection_ptr;

}

namespace zypp { namespace cache {

struct DatabaseContext
{
  sqlite3x::sqlite3_connection_ptr con;
  sqlite3x::sqlite3_command_ptr select_named_cmd;
  sqlite3x::sqlite3_command_ptr select_file_cmd;
  Pathname dbdir;
};

typedef zypp::shared_ptr<DatabaseContext> DatabaseContext_Ptr;

} }

#endif

