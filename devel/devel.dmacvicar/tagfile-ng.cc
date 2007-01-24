#include <iostream>
#include <fstream>
#include <sstream>
#include <streambuf>

#include "boost/filesystem/operations.hpp" // includes boost/filesystem/path.hpp
#include "boost/filesystem/fstream.hpp"    // ditto

#include <boost/iostreams/device/file_descriptor.hpp>

#include <zypp/base/Logger.h>
#include <zypp/Locale.h>
#include <zypp/ZYpp.h>
#include <zypp/ZYppFactory.h>
#include <zypp/TranslatedText.h>
#include <zypp/PathInfo.h>
#include <zypp/parser/tagfile/TagFileParser.h>
///////////////////////////////////////////////////////////////////

#include <zypp/parser/SAXParser.h>

using namespace zypp;
using namespace zypp::filesystem;

namespace experimental
{

class TagFileParser
{
public:
  struct MultiTag
  {
    std::string name;
    std::string modifier;
    std::list<std::string> values;
  };

  struct SingleTag
  {
    std::string name;
    std::string modifier;
    std::string value;
  };

  TagFileParser( ParserProgress::Ptr progress )
  {}
  
  virtual ~TagFileParser()
  {}

  virtual void parse( const Pathname & file_r)
  {
    _file_size = PathInfo(file_r).size();
    std::ifstream file(file_r.asString().c_str());
    int readed = 0;
  }

  virtual void beginParse();
  
  virtual void consume( const SingleTag &tag )
  {}
  
  virtual void consume( const MultiTag &tag )
  {}
  
  virtual void endParse();
        
protected:
  ParserProgress::Ptr _progress;
  Pathname _file_r;
  int _file_size;
  int _line_number;
};

}

int main( int argc, char * argv[] )
{
  myParser parser;
  parser.parseFile("repodata/primary.xml.gz");
}


