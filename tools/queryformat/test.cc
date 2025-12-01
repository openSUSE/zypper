#include <iostream>

#include "Parser.h"

using std::cout;
using std::cerr;
using std::endl;

//////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  namespace qf
  {
  } // namespace qf
} // namespace zypp


///////////////////////////////////////////////////////////////////
int main( int argc, const char ** argv )
{
  using namespace zypp;
  ++argv,--argc;
  if ( argc )
  {
    while ( argc ) {
      qf::Format result;
      if ( qf::parse( std::string_view(*argv), result ) )
        cout << "Parsed: " << result << endl;
      ++argv,--argc;
    }
  }
  else
  {
    for ( const char * qf : {
           "", "\\", "\\\\", "\n", "\\n", "\\%", "\\x",
           "%{NAME} %{SIZE}\\n",
           "%-{NAME} %-0009{SIZE}\\n",
           "[%-05{FILENAMES} %09{FILESIZES}\n]",
           "[%{NAME} %{FILENAMES}\\n]",
           "[%{=NAME} %{FILENAMES}\\n]",
           "%{NAME} %{INSTALLTIME:date}\\n",
           "[%{FILEMODES:perms} %{FILENAMES}\\n]",
           "[%{REQUIRENAME} %{REQUIREFLAGS:depflags} %{REQUIREVERSION}\\n]",
           "\\%\\x\\n%|SOMETAG?{present}:{missing}|",
    } )
    {
      qf::Format result;
      if ( qf::parse( qf, result ) )
        cout << "Parsed: " << result << endl;
    }
  }
  return 0;
}


