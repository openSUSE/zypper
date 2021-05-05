#include <zypp/PurgeKernels.h>
#include "argparse.h"

#define INCLUDE_TESTSETUP_WITHOUT_BOOST
#include "../tests/lib/TestSetup.h"
#undef  INCLUDE_TESTSETUP_WITHOUT_BOOST

static std::string appname { "NO_NAME" };

int errexit( const std::string & msg_r = std::string(), int exit_r = 100 )
{
  if ( ! msg_r.empty() )
    cerr << endl << appname << ": ERR: " << msg_r << endl << endl;
  return exit_r;
}

int usage( const argparse::Options & options_r, int return_r = 0 )
{
  cerr << "USAGE: " << appname << " [OPTION]... path/to/testcase" << endl;
  cerr << "    Calculate the kernels that would be purged based on the spec and testcase." << endl;
  cerr << options_r << endl;
  return return_r;
}

int main ( int argc, char *argv[] )
{

  appname = Pathname::basename( argv[0] );
  argparse::Options options;
  options.add()
    ( "help,h",   "Print help and exit." )
    ( "uname",    "The running kernels uname", argparse::Option::Arg::required )
    ( "keepSpec", "The keepspec ( default is oldest,running,latest)", argparse::Option::Arg::required );

  auto result = options.parse( argc, argv );

  if ( result.count( "help" ) || !result.count("uname") || !result.positionals().size() )
    return usage( options, 1 );

  const std::string &testcaseDir = result.positionals().front();
  const auto &pathInfo = PathInfo(testcaseDir);
  if ( !pathInfo.isExist() || !pathInfo.isDir() ) {
    std::cerr << "Invalid or non existing testcase path: " << testcaseDir << std::endl;
    return 1;
  }

  if ( ::chdir( testcaseDir.data() ) != 0 ) {
    std::cerr << "Failed to chdir to " << testcaseDir << std::endl;
    return 1;
  }

  TestSetup t;
  try {
    t.LoadSystemAt( result.positionals().front() );
  }  catch ( const zypp::Exception &e ) {
    std::cerr << "Failed to load the testcase at " << result.positionals().front() << std::endl;
    std::cerr << "Got exception: " << e << std::endl;
    return 1;
  }

  std::string keepSpec = "oldest,running,latest";
  if ( result.count("keepSpec") ) {
    keepSpec = result["keepSpec"].arg();
  }

  PurgeKernels krnls;
  krnls.setUnameR( result["uname"].arg() );
  krnls.setKeepSpec( keepSpec );
  krnls.markObsoleteKernels();

  const auto &makeNVRA = []( const PoolItem &pck ) -> std::string  {
    return pck.name() + "-" + pck.edition().asString() + "." + pck.arch().asString();
  };

  auto pool = ResPool::instance();
  const filter::ByStatus toBeUninstalledFilter( &ResStatus::isToBeUninstalled );
  for ( auto it = pool.byStatusBegin( toBeUninstalledFilter ); it != pool.byStatusEnd( toBeUninstalledFilter );  it++  ) {
    std::cout << "Removing " << makeNVRA(*it) + (it->status().isByUser() ? " (by user)" : " (autoremoved)") << std::endl;
  }

  return 0;

}
