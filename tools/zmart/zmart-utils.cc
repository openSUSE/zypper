#include <fstream>
#include <sstream>
#include <boost/format.hpp>
#include "zmart.h"
#include "zmart-misc.h"
#include "zypper-tabulator.h"

#include <zypp/Patch.h>

using namespace zypp::detail;

using namespace std;
using namespace zypp;

string read_line_from_file( const Pathname &file )
{
  string buffer;
  string token;
  std::ifstream is(file.asString().c_str());
  if ( is.good() )
  {
    while(is && !is.eof())
    {
      getline(is, buffer);
      token += buffer;
    }
    is.close();
  }
  return token;
 }


void write_line_to_file( const Pathname &file, const std::string &line )
{
  std::ofstream os(file.asString().c_str());
  if ( os.good() )
  {
    os << line << endl;;
  }
  os.close();
}
