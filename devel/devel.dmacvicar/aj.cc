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
#include <zypp/SourceManager.h>
///////////////////////////////////////////////////////////////////

#include <zypp/parser/yum/YUMParser.h>
#include <zypp/base/Logger.h>
#include <zypp/source/yum/YUMScriptImpl.h>
#include <zypp/source/yum/YUMMessageImpl.h>
#include <zypp/source/yum/YUMPackageImpl.h>
#include <zypp/source/yum/YUMSourceImpl.h>

#include <map>
#include <set>

#include <zypp/CapFactory.h>
#include <zypp/base/LogTools.h>

using namespace zypp::detail;

using namespace std;
using namespace zypp;
using namespace zypp::parser::yum;
using namespace zypp::source::yum;


static std::string dump( const ResStore &store )
{
  stringstream out;
  for (ResStore::const_iterator it = store.begin(); it != store.end(); it++)
  {
    MIL << **it << endl;
    if ( isKind<Patch>(*it) )
    {
      MIL << asKind<Patch>(*it)->atoms().size() << std::endl;
    }
    MIL << "------------------------------------------------" << endl;
    //MIL << (**it).deps() << endl;
  }
  return out.str();
}

//using namespace DbXml;

int main()
{
  try
  { 
    ZYpp::Ptr z = getZYpp();
    z->initTarget(Pathname("/"));
    
    ResStore s = z->target()->resolvables();
    
    dump(s);
    
    z->addResolvables(s, true);
    
    
    //printRange( z->pool().begin(), z->pool().end(), MIL ) << std::endl;
    
    SourceManager_Ptr manager = SourceManager::sourceManager();
    try {
      manager->restore( "/" );
      MIL << "Sources restored." << std::endl;
    }
    catch (Exception & excpt_r) {
      ZYPP_CAUGHT( excpt_r );
      ERR << "Couldn't restore sources" << std::endl;
      exit( 1 );
    }
    
    for (SourceManager::Source_const_iterator it = manager->Source_begin(); it !=  manager->Source_end(); ++it) {
      zypp::ResStore store = it->resolvables();
      MIL << "Repository " << it->id() << " contributing " << store.size() << " resolvables" << endl;
      z->addResolvables( store, false );
    }  
  
    //printRange( z->pool().begin(), z->pool().end(), MIL ) << std::endl;
    
  }
  catch (...)
  {
    MIL << "sorry" << std::endl;
  } 
}


