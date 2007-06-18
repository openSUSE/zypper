#include <iostream>
#include "zypp/base/Logger.h"
#include "zypp/base/LogTools.h"
#include "zypp/base/Function.h"
#include "zypp/base/IOStream.h"
#include "zypp/base/InputStream.h"
#include "zypp/ProgressData.h"

#include "zypp/base/Random.h"

#include <boost/thread.hpp>

using std::endl;
using namespace zypp;


void action( int i_r )
{
  unsigned sec = base::random( 3 );
  sleep( sec );
  MIL << "Action " << i_r << " (" << sec << ")" << endl;
}


int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;

  for ( unsigned i = 0; i < 5; ++i )
  {
    new boost::thread( bind( action, i ) );
  }

  INT << "===[END]============================================" << endl << endl;
  return ( 0 );
}
