
#include <string>

#include "zypp/base/Logger.h"
#include "zypp/base/GzStream.h"
#include "zypp/Url.h"
#include "zypp/PathInfo.h"
#include "zypp/TmpPath.h"

#include "SimplePackagesParser.h"


using namespace zypp;
using namespace std;

static void read_dash( ifgzstream &ifs, const std::string &s, int &line )
{
  string buffer;
  // get the "-"
  getline(ifs, buffer);
  line++;
  if ( buffer != s )
  {
    ERR << "line : " << line << endl;
    ZYPP_THROW(Exception("missing " + s ));
  }
}

static void read_deps( ifgzstream &ifs, data::DependencyList &list, int &line, const string &endchar )
{
  string buffer;
  while ( ifs && !ifs.eof())
  {
    getline(ifs, buffer);
    line++;
    if ( buffer == endchar )
      break;
    try
    {
      capability::CapabilityImpl::Ptr cap = capability::parse( ResTraits<Package>::kind, buffer);
      if (cap)
        list.push_back(cap);
    }
    catch( const Exception  &e )
    {
      ERR << "line : " << line << endl;
      ZYPP_THROW(Exception("bad capability line")); 
    }    
  }
}

void parse_mini_file(const Pathname &nvra_list, std::list<MiniResolvable> &res_list)
{
  std::string buffer;
  int line = 0;
  ifgzstream nvra_stream(nvra_list.c_str());
  MIL << "reading " << nvra_list << endl;
  
  if ( ! nvra_stream )
    ZYPP_THROW(Exception("cant open data file " + nvra_list.asString()));
  
  while ( nvra_stream && !nvra_stream.eof())
  {
    MiniResolvable res;
    getline(nvra_stream, buffer);
    line++;
    
    if ( buffer.empty() )
      break;
    
    std::vector<std::string> words;
    if ( str::split( buffer, std::back_inserter(words) ) != 4 )
    {
      ERR << nvra_list << " : line : " << line << endl;
      ZYPP_THROW(Exception("bad NVRA line"));
    }
    
    res.nvra = NVRA(words[0], Edition(words[1], words[2]), Arch(words[3]));
    // requires
    read_dash( nvra_stream, "+r", line);
    read_deps( nvra_stream, res.deps[Dep::REQUIRES], line, "-r");
    read_dash( nvra_stream, "+p", line);
    read_deps( nvra_stream, res.deps[Dep::PROVIDES], line, "-p");
    
    res_list.push_back(res);
  }
  //MIL << deps.size() << " capabilities read." << endl;
}

