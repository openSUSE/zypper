#include <iostream>
#include <fstream>

#include <boost/filesystem/path.hpp>

#define DBG std::cout
#define MIL std::cout
#define WAR std::cerr
#define ERR std::cerr

using std::endl;

///////////////////////////////////////////////////////////////////
namespace parse
{ /////////////////////////////////////////////////////////////////

  std::string getline( std::istream & str )
  {
    static const unsigned tmpBuffLen = 1024;
    static char tmpBuff[tmpBuffLen];
    std::string ret;
    do {
      str.clear();
      str.getline( tmpBuff, tmpBuffLen ); // always writes '\0' terminated
      ret += tmpBuff;
    } while( str.rdstate() == std::ios::failbit );

    return ret;
  }

  /////////////////////////////////////////////////////////////////
} // namespace parse
///////////////////////////////////////////////////////////////////


/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  DBG << "===[START]==========================================" << endl;




  DBG << "===[END]============================================" << endl;
  return 0;
}

