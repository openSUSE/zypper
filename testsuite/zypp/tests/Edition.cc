// Edition.cc
//
// tests for Edition
//

#include "zypp/base/Logger.h"
#include "zypp/Edition.h"

using namespace std;
using namespace zypp;

static int
edition_exception (const std::string &value)
{
  try {
    Edition _ed(value);	// bad value, should raise exception
  } catch (exception exp) {
    return 0;		// exception raised
  }
  return 1;		// no exception
}

/******************************************************************
**
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
**
**      DESCRIPTION :
*/
int main( int argc, char * argv[] )
{
  if (edition_exception (string("A::foo--foo"))) return 1;
  Edition _ed1 ("1");
  Edition _ed2 ("1.1");
  Edition _ed3 ("1:1");
  Edition _ed4 ("1:1-1");

  if (_ed2.version() != "1.1") return 2;
  if (_ed2.release() != "") return 3;
  if (_ed2.epoch() != 0) return 4;
  if (_ed4.epoch() != 1) return 5;

  if (_ed1 != Edition ("1", "")) return 6;
  if (_ed2 != Edition ("1.1", "")) return 7;
  if (_ed3 != Edition ("1", "", "1")) return 8;
  if (_ed3 != Edition ("1", "", 1)) return 9;
  if (_ed4 != Edition ("1", "1", 1)) return 10;

  return 0;
}
