#include <iostream>
#include <zypp/CpeId.h>
#include <zypp/Pathname.h>

using std::cout;
using std::endl;
using zypp::CpeId;
using zypp::Pathname;

int main( int argc, const char * argv[] )
{
  if ( argc == 1 || argv[1] == std::string( "--help" ) || argv[1] == std::string( "-h" ) )
  {
    cout <<
    "Usage: " << Pathname::basename( argv[0] ) << " [CPEID]...\n"
    "Check and print all supplied CPEIDs as FS, URI and WFN.\n"
    "Afterwards compare them pairwise. \n"
    "\n"
    "      (wfn:[part=\"a\",vendor=\"openSUSE\",product=\"libzypp\",version=\"14\\.17\\.3\"])\n"
    "  URI: cpe:/a:openSUSE:libzypp:14.17.3\n"
    "  FS:  cpe:2.3:a:openSUSE:libzypp:14.17.3:*:*:*:*:*:*:*\n"
    "\n";

    return 0;
  }
  --argc, ++argv;


  std::vector<CpeId> args;
  args.reserve( argc );

  for ( ; argc; --argc, ++argv )
  {
    try {
      CpeId cpe( argv[0] );
      cout << '[' << args.size() << "]-----------------------------------------------------------------------------" << endl;
      cout << "arg: " << argv[0] << endl;
      cout << "    (" << cpe.asWfn() << ')' << endl;
      cout << "URI: " << cpe.asUri() << endl;
      cout << "FS:  " << cpe<< endl;
      args.push_back( cpe );
    }
    catch ( const std::invalid_argument & exp )
    {
      cout << "--------------------------------------------------------------------------------" << endl;
      cout << "arg: " << argv[0] << endl;
      cout << "ERR: " << exp.what() << endl;
    }
  }

  cout << "--------------------------------------------------------------------------------" << endl;
  unsigned lhsidx = 0;
  for ( const auto & lhs : args )
  {
    unsigned rhsidx = 0;
    for ( const auto & rhs : args )
    {
      cout << "[" << lhsidx << "]  " << lhs << endl;
      cout << "[" << rhsidx << "]  " << rhs << endl;
      cout << " ==> " << compare( lhs, rhs ) << endl;
      ++rhsidx;
    }
    ++lhsidx;
  }
  return 0;
}
