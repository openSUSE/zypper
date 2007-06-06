/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */

#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/ZYpp.h"
#include "zypp/zypp_detail/ZYppReadOnlyHack.h"
#include "zypp/ZYppFactory.h"
#include "zypp/cache/CacheFSCK.h"

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "zypp-cache"


using namespace std;
using namespace zypp;

static void cache_fsck( const Pathname &dir )
{
  cache::CacheFSCK fsck(dir);
  fsck.start();
}

void usage()
{
  cout << "Commands:" << endl;
  cout << "fsck dbdir" << endl;
}

//-----------------------------------------------------------------------------

int
main (int argc, char **argv)
{
  MIL << "-------------------------------------" << endl;
  
  if (argc > 2)
  {
    if ( string(argv[1]) == "fsck" )
    {
      cache_fsck(argv[2]);
    }
    else
    {
      usage();
    }
  }
  else
  {
    usage();
  }

  return 0;
}
