#include "Tools.h"
#include <zypp/ResObjects.h>

#include <zypp/sat/LookupAttr.h>

  static std::string pidAndAppname()
  {
    static std::string _val;
    if ( _val.empty() )
    {
      pid_t mypid = getpid();
      Pathname p( "/proc/"+str::numstring(mypid)+"/exe" );
      Pathname myname( filesystem::readlink( p ) );

      _val += str::numstring(mypid);
      _val += ":";
      _val += myname.basename();
    }
    return _val;
  }

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;

  SEC << pidAndAppname() << endl;


  INT << "===[END]============================================" << endl << endl;
  return 0;
  Pathname mroot( "/tmp/Bb" );
  TestSetup test( mroot, Arch_x86_64 );
  test.loadRepo( "/Local/ROOT/cache/raw/11.1-update" );
  test.loadRepo( "/Local/ROOT/cache/raw/11.0-update" );

  sat::Pool satpool( test.satpool() );
  for_( it, satpool.reposBegin(), satpool.reposEnd() )
  {
    MIL << *it << endl;
    DBG << it->generatedTimestamp() << endl;
    DBG << it->suggestedExpirationTimestamp() << endl;
  }

  INT << "===[END]============================================" << endl << endl;
  return 0;

  sat::LookupRepoAttr q( sat::SolvAttr::repositoryAddedFileProvides );
  USR << q << endl;
  USR << dump(q) << endl;
  for_( it, q.begin(), q.end() )
  {
    MIL << it << endl;
  }



  INT << "===[END]============================================" << endl << endl;
  return 0;
}

