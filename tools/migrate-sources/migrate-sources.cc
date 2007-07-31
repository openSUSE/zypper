/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */

#include <iostream>
#include <fstream>

#include "zypp/base/Logger.h"
#include "zypp/ZYpp.h"
#include "zypp/zypp_detail/ZYppReadOnlyHack.h"
#include "zypp/ZYppFactory.h"
#include "zypp/PathInfo.h"
#include "zypp/RepoManager.h"
#include "zypp/cache/CacheFSCK.h"

#include "zypp/parser/xmlstore/XMLSourceCacheParser.h"

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "zypp-cache"


using namespace std;
using namespace zypp;

static void migrate_sources( const Pathname &root, const Pathname &dir )
{
  RepoManager manager;
  
  Pathname source_p = root + dir;
  
  RepoInfoList sources;
  DBG << "Reading source cache in " << source_p << std::endl;
  
  list<Pathname> entries;
  if ( filesystem::readdir( entries, source_p, false ) != 0 )
      ZYPP_THROW(Exception("failed to read directory"));

  int i=0;
  for ( list<Pathname>::const_iterator it = entries.begin(); it != entries.end(); ++it )
  {
    
    MIL << "Processing " << *it << endl;
    
    std::ifstream anIstream((*it).c_str());
    zypp::parser::xmlstore::XMLSourceCacheParser iter(anIstream, "");
    for (; ! iter.atEnd(); ++iter) {
      RepoInfo data = **iter;
      string alias = "migrated_" + str::numstring(i);
      try {
        cout << "Migrating repo: " << endl << data << endl;
        data.setAlias(alias);
        data.setEnabled(false);
        manager.addRepository(data);
        cout << "saved as " << alias << endl;
        ++i;
      }
      catch ( const Exception &e )
      {
        cout << "Error adding repository: " << e.msg() << endl << data << endl;
      }
      
    }

  }
  cout << i << " sources migrated."<< endl;
}

void usage(int argc, char **argv)
{
  cout << argv[0] << ". Migrates old sources to 10.3 repositories." << endl;
  cout << "Usage:" << endl;
  cout << argv[0] << " root-path" << endl;
}

//-----------------------------------------------------------------------------

int
main (int argc, char **argv)
{
  MIL << "-------------------------------------" << endl;
  
  if (argc > 1)
  {
      migrate_sources(argv[1], "/var/lib/zypp/db/sources");
  }
  else
  {
    usage(argc, argv);
  }

  return 0;
}

