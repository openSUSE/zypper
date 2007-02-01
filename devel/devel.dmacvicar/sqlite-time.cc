#include <sys/time.h>

#include <iostream>
#include <fstream>

#include "sqlite3.h"
#include "zypp2/cache/sqlite3x/sqlite3x.hpp"

#include <zypp/base/Logger.h>
#include <zypp/base/Measure.h>
#include <zypp/ZYpp.h>
#include <zypp/ZYppFactory.h>

#include "zypp/TmpPath.h"
#include "zypp/Product.h"
#include "zypp/detail/PackageImplIf.h"
#include "zypp/Package.h"

#include "zypp/data/ResolvableData.h"

using namespace std;
using namespace zypp;
using namespace sqlite3x;

#define CREATE_QUERY "CREATE TABLE dependencies ( \
    id INTEGER PRIMAR KEY AUTO INCREMENT \
  , dep_type INTEGER \
  , name TEXT \
  , version TEXT \
  , release TEXT \
  , epoch INTEGER \
  , arch INTEGER \
  , relation INTEGER \
  , dep_target INTEGER \
);"

#define INSERT_QUERY "insert into dependencies (dep_type, name, version, release, epoch, arch, relation, dep_target) values (?,?,?,?,?,?,?,?);"
#define SELECT_QUERY "select name,version,release,epoch from dependencies"

#define ITER 20000
int main(int argc, char **argv)
{
    try
    {
      filesystem::TmpDir tmpdir;
      MIL << "Db will be in " << tmpdir << endl;
      
      sqlite3_connection con((tmpdir.path() + "test.db").asString().c_str() );
      sqlite3_command createcmd(con, CREATE_QUERY);
      createcmd.executenonquery();
      
      zypp::debug::Measure m("sqlite3x insert");
      sqlite3_transaction trans(con);
      {
        sqlite3_command insertcmd(con, INSERT_QUERY);
        for (int i =0; i < ITER; i++ )
        {
          insertcmd.bind(1, 1);
          insertcmd.bind(2, "name", 4);
          insertcmd.bind(3, "version", 7);
          insertcmd.bind(4, "release", 7);
          insertcmd.bind(5, 0);
          insertcmd.bind(6, 3);
          insertcmd.bind(7, 2);
          insertcmd.bind(8, 1);
          insertcmd.executenonquery();
        }
      }
      m.elapsed();
      m.stop();
      
      zypp::debug::Measure ms("sqlite3x select");
      sqlite3_command selectcmd(con, "select name,version,release,epoch from dependencies");
      sqlite3_reader reader = selectcmd.executereader();

       while(reader.read())
       {
          string a = reader.getcolname(0);
          int b = reader.getint(3);
       }
       
       ms.elapsed();
       ms.stop();
    }
    catch ( const Exception &e )
    {
      ZYPP_CAUGHT(e);
    }
    
    try
    {
      filesystem::TmpDir tmpdir;
      MIL << "Db 2 will be in " << tmpdir << endl;
      
      sqlite3 *db;
      int rc = sqlite3_open ( (tmpdir.path() + "test.db").asString().c_str(), &db );
      if (rc != SQLITE_OK || db == NULL )
      {
        ZYPP_THROW(Exception(sqlite3_errmsg(db)));
      }
      
      sqlite3_exec( db, CREATE_QUERY, 0, 0, 0);
      
      zypp::debug::Measure m("sqlite-c insert");
      
      sqlite3_stmt *handle = 0;
      rc = sqlite3_prepare( db, INSERT_QUERY, -1, &handle,0);
      
      sqlite3_exec( db, "BEGIN;", 0, 0, 0);
      for (int i =0; i < ITER; i++ )
      {
        rc = sqlite3_bind_int( handle, 1, 1);
        rc = sqlite3_bind_text( handle, 2, "name", -1, SQLITE_STATIC );
        rc = sqlite3_bind_text( handle, 3, "version", -1, SQLITE_STATIC );
        rc = sqlite3_bind_text( handle, 4, "release", -1, SQLITE_STATIC );
        rc = sqlite3_bind_int( handle, 5, 0);
        rc = sqlite3_bind_int( handle, 6, 3);
        rc = sqlite3_bind_int( handle, 7, 2);
        rc = sqlite3_bind_int( handle, 8, 1);
        if ( sqlite3_step(handle) != SQLITE_DONE )
        {
          ZYPP_THROW(Exception("error on insert"));
        }
        sqlite3_reset (handle);
      }
      sqlite3_exec( db, "COMMIT;", 0, 0, 0);
      m.elapsed();
      m.stop();
      
      zypp::debug::Measure ms("sqlite-c select");
      
      rc = sqlite3_prepare( db, SELECT_QUERY, -1, &handle,0);

       while( sqlite3_step(handle) == SQLITE_ROW )
       {
          string a = (const char *) sqlite3_column_text( handle, 0 );
          int b = sqlite3_column_int( handle, 3 );
       }
       
       sqlite3_finalize (handle);
       ms.elapsed();
       ms.stop();
    }
    catch ( const Exception &e )
    {
      ZYPP_CAUGHT(e);
    }
    
    return 0;
}



