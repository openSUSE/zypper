#include <iostream>

#include "Tools.h"

#include <string>
#include <ext/hash_set>
#include <ext/hash_fun.h>

using namespace std;
using namespace zypp;

///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
namespace __gnu_cxx
{ /////////////////////////////////////////////////////////////////

  template<>
    struct hash<std::string>
    {
      size_t operator()( const std::string & key_r ) const
      { return __stl_hash_string( key_r.c_str() ); }
    };

  /////////////////////////////////////////////////////////////////
} // namespace __gnu_cxx
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////



  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  //zypp::base::LogControl::instance().logfile( "" );
  INT << "===[START]==========================================" << endl;

  typedef __gnu_cxx::hash_set<std::string> HSet;
  
  HSet hset;
  
  
  
  
  DBG << __gnu_cxx::hash<const char*>()( "" ) << endl;
  DBG << __gnu_cxx::hash<const char*>()( "a" ) << endl;
  DBG << __gnu_cxx::hash<const char*>()( "aa" ) << endl;
  DBG << __gnu_cxx::hash<const char*>()( "ab" ) << endl;
  DBG << __gnu_cxx::hash<const char*>()( "ac" ) << endl;
  DBG << __gnu_cxx::hash<const char*>()( "b" ) << endl;
  DBG << __gnu_cxx::hash<const char*>()( "c" ) << endl;
  DBG << __gnu_cxx::hash<const char*>()( "" ) << endl;
  DBG << __gnu_cxx::hash<std::string>()( std::string("foo") ) << endl;


  INT << "===[END]============================================" << endl << endl;
  new zypp::base::LogControl::TmpLineWriter;
  return 0;
}

