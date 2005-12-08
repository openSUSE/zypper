#include <iostream>
#include "zypp/base/Logger.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/base/Exception.h"

using namespace std;
using namespace zypp;

class MediaException : public Exception
{
  public:
    MediaException( const std::string & msg_r )
    : Exception( msg_r )
    {}
};

void ex()
{
  try
    {
      ZYPP_THROW( Exception, "plain exeption" );
    }
  catch ( Exception & excpt )
    {
      ZYPP_CAUGHT( excpt );
    }
}

void mex()
{
  try
    {
      ZYPP_THROW( MediaException, "media error" );
    }
  catch ( MediaException & excpt )
    {
      SEC << "CAUGHT MediaException" << endl;
      ZYPP_CAUGHT( excpt );
    }
  catch ( Exception & excpt )
    {
      ZYPP_CAUGHT( excpt );
    }
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
  INT << "===[START]==========================================" << endl;

  ex();
  mex();

  INT << "===[END]============================================" << endl;
  return 0;
}
