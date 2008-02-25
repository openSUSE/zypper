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

struct Options
{
  Options()
  : fake(false)
  , root("/")
  , sources_dir("/var/lib/zypp/db/sources")
  {}

  bool fake;
  Pathname root;
  Pathname sources_dir;
};

static void clear_cache( const Options &opt )
{
  Pathname path = opt.root + "/var/lib/zypp/cache";
  if ( PathInfo(path).isDir() )
  {
    cout << "Deleting old cache directory (" << path << ")." << endl;
    if ( ! opt.fake )
    {
      if ( filesystem::recursive_rmdir(path) != 0 )
        ERR << "Error removing cache directory" << path << endl;
    }
  }

  path = opt.root + "/var/lib/zypp/db";
  if ( PathInfo(path).isDir() )
  {
    cout << "Deleting old db directory (" << path << ")." << endl;
    if ( ! opt.fake )
    {
      if ( filesystem::recursive_rmdir(path) != 0 )
        ERR << "Error removing db directory" << path << endl;
    }
  }
}

static void migrate_sources( const Options &opt )
{
  if ( getenv("YAST_IS_RUNNING") && (string(getenv("YAST_IS_RUNNING")) == "instsys" ))
  {
    MIL << "YaST is running in instsys. Not migrating old sources. YaST will do it." << endl;
    return;
  }
  else
  {
    MIL << "YaST not running in instsys." << endl;
  }

  zypp::zypp_readonly_hack::IWantIt();
  ZYpp::Ptr Z = zypp::getZYpp();
  RepoManager manager;

  Pathname source_p = opt.root + opt.sources_dir;

  if ( ! PathInfo(source_p).isExist() )
  {
    cout << "No sources to migrate." << endl;
    clear_cache( opt );
    return;
  }

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
        data.setAlias(alias);
        data.setEnabled(false);
        cout << "Migrating repo: " << endl << data << endl;
        if ( ! opt.fake )
        {
          manager.addRepository(data);
        }
        cout << "Deleting old source: " << *it << endl;
        if ( ! opt.fake )
        {
          if ( filesystem::unlink(*it) != 0 )
            ERR << "Error removing source " << *it << endl;
          // delete old file
        }
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

  // reread entries
  if ( filesystem::readdir( entries, source_p, false ) != 0 )
      ZYPP_THROW(Exception("failed to read directory"));
  if ( entries.size() == 0 )
  {
    cout << "all sources migrated. deleting old source directory"<< endl;
    if ( ! opt.fake )
    {
      if ( filesystem::recursive_rmdir(source_p) != 0 )
        ERR << "Error removing source directory" << source_p << endl;

      clear_cache( opt );
    }
  }
  else
  {
    cout << "Not all sources migrated. leaving old source directory"<< endl;
  }
}

void usage(int argc, char **argv)
{
  cout << argv[0] << ". Migrates old sources to 10.3 repositories." << endl;
  cout << "Usage:" << endl;
  cout << argv[0] << " [--root root-path] [--fake] [--sp sources-path]" << endl;
}

//-----------------------------------------------------------------------------

int
main (int argc, char **argv)
{
  MIL << "-------------------------------------" << endl;
  Options opt;
  int i;
  for ( i=1; i < argc; ++i )
  {

    if ( string(argv[i]) == "--help" )
    {
      usage(argc, argv);
      return 0;
    }
    if ( string(argv[i]) == "--fake" )
      opt.fake = true;
    if ( string(argv[i]) == "--root" )
      opt.root = argv[++i];
    if ( string(argv[i]) == "--sp" )
      opt.sources_dir = argv[++i];
  }
  migrate_sources(opt);

  return 0;
}

