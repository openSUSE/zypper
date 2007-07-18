#include <iostream>

#include "zypp/base/Easy.h"
#include "zypp/base/LogTools.h"
#include "zypp/base/InputStream.h"

#include "zypp/parser/IniDict.h"

using std::endl;
using namespace zypp;

///////////////////////////////////////////////////////////////////

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;

  Pathname file( "test.ini" );
  InputStream is( file );
  parser::IniDict dict( is );

  SEC << endl;
  for_( it, dict.sectionsBegin(), dict.sectionsEnd() )
  {
    MIL << (*it) << endl;

    for_( ent, dict.entriesBegin(*it), dict.entriesEnd(*it) )
    {
      DBG << "'" << (*ent).first << "'='" << (*ent).second << "'" << endl;
    }
  }
  SEC << endl;

  INT << "===[END]============================================" << endl << endl;
  return 0;
}

