#include <iostream>
#include <fstream>
#include <sstream>
#include <streambuf>
 #include <ctime>

#include "boost/filesystem/operations.hpp" // includes boost/filesystem/path.hpp
#include "boost/filesystem/fstream.hpp"    // ditto

#include <boost/iostreams/device/file_descriptor.hpp>

#include "zypp/SourceFactory.h"

#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
///////////////////////////////////////////////////////////////////

#include "zypp/source/SourceInfo.h"
#include "zypp/target/store/PersistentStorage.h"
#include "zypp/target/store/XMLFilesBackend.h"
#include "zypp/parser/tagfile/TagFileParser.h"

#include "zypp/base/Logger.h"

#include "zypp/SourceFactory.h"
#include "zypp/Source.h"
#include "zypp/source/SourceImpl.h"
#include "zypp/PathInfo.h"

#include <map>
#include <set>

#include "zypp/CapFactory.h"

#include "zypp/target/store/serialize.h"

using namespace zypp::detail;
using namespace std;
using namespace zypp;
using namespace zypp::storage;
using namespace zypp::source;

using namespace boost::filesystem;

using namespace zypp::parser;

void progress( int p )
{
  cout << p << "%" << endl;
}

/*
=Type: UnitedLinux
=URL: ftp://dist.suse.de//next-i386
=ProductDir: /
=Default_activate: 0
=Default_refresh: 1
=Default_rank: 2
=UseForDeltas: 1
*/

/*
  bool enabled;
  bool autorefresh;
  std::string product_dir;
  std::string type;
  std::string url;
  std::string cache_dir;
  std::string alias;
*/
struct OldPMSourceParser : public parser::tagfile::TagFileParser
{
  source::SourceInfo result;
  
  OldPMSourceParser( ParserProgress::Ptr pp ) : tagfile::TagFileParser( pp )
  {
    
  }
  
  virtual void beginParse()
  {
  }
  
  virtual void consume( const SingleTag & stag_r )
  {
    if ( stag_r.name == "Type" )
      result.setType(stag_r.value);
    if ( stag_r.name == "URL" )
    {
      result.setUrl(stag_r.value);
      result.setAlias(stag_r.value);
    }
    if ( stag_r.name == "ProductDir" )
      result.setPath(stag_r.value);
    if ( stag_r.name == "Default_activate" )
      result.setEnabled( (stag_r.value == "1") ? true : false );
    if ( stag_r.name == "Default_refresh" )
      result.setAutorefresh( (stag_r.value == "1") ? true : false );
  }
  
  /* Consume MulitTag data. */
  virtual void consume( const MultiTag & mtag_r )
  {
  }
  
  virtual void endParse()
  {
          
  }
};

static int import_old_sources()
{
  Pathname root("."); 
  XMLFilesBackend backend(root);
  
  directory_iterator end_iter;
  path dir_path("/var/adm/YaST/InstSrcManager/");
  // return empty list if the dir does not exist
  if ( !exists( dir_path ) )
    ZYPP_THROW(Exception("Can't find " + dir_path.string()
                        ));
  for ( directory_iterator dir_itr( dir_path ); dir_itr != end_iter; ++dir_itr )
  {
    if (! dir_itr->leaf().find("IS_CACHE_") )
    {
      try
      {
        DBG << dir_itr->leaf() << std::endl;
        Pathname full(Pathname(dir_path.string()) + dir_itr->leaf() + "/DESCRIPTION/description");
        ParserProgress::Ptr pptr;
        pptr.reset( new ParserProgress( &progress ) );
        OldPMSourceParser parser( pptr );
        MIL << "Going to parse " << full << std::endl;
        parser.parse(full);
        backend.storeSource(parser.result);
      }
      catch ( Exception &e )
      {
        ZYPP_RETHROW(e);
      }
    }
  }
  
  MIL << "done reading " << dir_path.string() << std::endl;
  return 0;
}

int main()
{
  int error = 0;
  if ((error = import_old_sources()) != 0) return error;
  return 0;
}
