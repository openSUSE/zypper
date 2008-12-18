#include "Tools.h"
#include <zypp/ResObjects.h>

#include <zypp/sat/LookupAttr.h>
#include <zypp/PoolQuery.h>

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

bool solve()
{
  static unsigned run = 0;
  USR << "Solve " << run++ << endl;
  bool rres = false;
  {
    zypp::base::LogControl::TmpLineWriter shutUp;
    rres = getZYpp()->resolver()->resolvePool();
  }
  if ( ! rres )
  {
    ERR << "resolve " << rres << endl;
    getZYpp()->resolver()->problems();
    return false;
  }

  return true;
}

typedef sat::ArrayAttr<std::string,std::string> FileList;

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;

  Pathname mroot( "/tmp/ToolScanRepos" );
  TestSetup test( mroot, Arch_x86_64 );
  test.loadRepo("/Local/ROOT/cache/solv/@System/solv");

  ResPool pool( test.pool() );
  {
    Measure x("filelist");
    unsigned p = 0;
    unsigned f = 0;
    std::string a;
    for_( it, pool.byKindBegin<Package>(), pool.byKindEnd<Package>() )
    {
      ++p;
      f += (*it)->asKind<Package>()->filelist().size();
      for_( i, (*it)->asKind<Package>()->filelist().begin(), (*it)->asKind<Package>()->filelist().end() )
        a = *i;
    }
    SEC << p << " : " << f << endl;
  }
  {
    Measure x("filenames");
    unsigned p = 0;
    unsigned f = 0;
    std::string a;
    for_( it, pool.byKindBegin<Package>(), pool.byKindEnd<Package>() )
    {
      ++p;
      std::list<std::string> l( (*it)->asKind<Package>()->filenames() );
      f += l.size();
      for_( i, l.begin(), l.end() )
        a = *i;
    }
    SEC << p << " : " << f << endl;
  }

  INT << "===[END]============================================" << endl << endl;
  return 0;



  //ui::Selectable::Ptr getSel( const std::string & name_r )
  getSel<Package>( "gcompris" )->setToInstall();

  vdumpPoolStats( USR << "Transacting:"<< endl,
                  make_filter_begin<resfilter::ByTransact>(pool),
                  make_filter_end<resfilter::ByTransact>(pool) ) << endl;

  if ( solve() )
  {
    vdumpPoolStats( USR << "Transacting:"<< endl,
                    make_filter_begin<resfilter::ByTransact>(pool),
                    make_filter_end<resfilter::ByTransact>(pool) ) << endl;
    SEC << getSel<Package>( "librsvg" ) << endl;
  }
  INT << "===[END]============================================" << endl << endl;
  return 0;
}

