#include <iostream>
#include "zypp/base/Logger.h"
#include "zypp/target/rpm/RpmDb.h"


using namespace std;
using namespace zypp;
using namespace zypp::target::rpm;

/******************************************************************
**
**
**	FUNCTION NAME : main
**	FUNCTION TYPE : int
**
**	DESCRIPTION :
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;

  RpmDb db;

  DBG << "===[DB OBJECT CREATED]==============================" << endl;

  db.initDatabase();

  DBG << "===[DATABASE INITIALIZED]===========================" << endl;

  std::list<Package::Ptr> packages = db.getPackages();
  for (std::list<Package::Ptr>::const_iterator it = packages.begin();
       it != packages.end();
       it++)
  {
//    DBG << **it << endl;
  }

  INT << "===[END]============================================" << endl;
  return 0;
}
