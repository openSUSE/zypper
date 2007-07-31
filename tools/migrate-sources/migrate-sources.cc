/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */

#include <iostream>
#include <fstream>

#include "zypp/base/Logger.h"
#include "zypp/ZYpp.h"
#include "zypp/zypp_detail/ZYppReadOnlyHack.h"
#include "zypp/ZYppFactory.h"
#include "zypp/PathInfo.h"
#include "zypp/cache/CacheFSCK.h"

#include "zypp/parser/xmlstore/XMLSourceCacheParser.h"

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "zypp-cache"


using namespace std;
using namespace zypp;

static void migrate_sources( const Pathname &root, const Pathname &dir )
{
  Pathname source_p = root + dir;
  
  RepoInfoList sources;
  DBG << "Reading source cache in " << source_p << std::endl;
  
  list<Pathname> entries;
  if ( filesystem::readdir( entries, source_p, false ) != 0 )
      ZYPP_THROW(Exception("failed to read directory"));

  for ( list<Pathname>::const_iterator it = entries.begin(); it != entries.end(); ++it )
  {
    MIL << "Processing " << *it << endl;
    
    std::ifstream anIstream((*it).c_str());
    zypp::parser::xmlstore::XMLSourceCacheParser iter(anIstream, "");
    for (; ! iter.atEnd(); ++iter) {
      RepoInfo data = **iter;
      cout << data << endl << endl << endl;
    }

  }
}

void usage()
{
  cout << "Commands:" << endl;
  cout << "migrate-sources root-path" << endl;
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
    usage();
  }

  return 0;
}
