extern "C"
{
#include <stdio.h>
#include <satsolver/pool.h>
#include <satsolver/repo_solv.h>
}
#include <cstdio>
#include <iostream>

#include "zypp/Pathname.h"
#include "zypp/base/Logger.h"
#include "zypp/base/LogControl.h"

using std::endl;
using namespace zypp;

int main( int argc, char * argv[] )
{
  //zypp::base::LogControl::instance().logfile( "log.restrict" );
  INT << "===[START]==========================================" << endl;

  ::_Pool * _pool = ::pool_create();

  Pathname p;

  p = "sl10.1-beta7-selections.solv";
  FILE * file = ::fopen( p.c_str(), "r" );
  ::pool_addrepo_solv( _pool, file, p.c_str() );
  ::fclose( file );

  p = "1234567890.solv";
  file = ::fopen( p.c_str(), "r" );
  ::pool_addrepo_solv( _pool, file, p.c_str() );
  ::fclose( file );

  p = "sl10.1-beta7-packages.solv";
  file = ::fopen( p.c_str(), "r" );
  ::pool_addrepo_solv( _pool, file, p.c_str() );
  ::fclose( file );

  ::_Pool & pool( *_pool );

  MIL << _pool->nrepos << endl;
  MIL << (void*)(*pool.repos) << " " << (*pool.repos)->name << endl;
  MIL << (void*)(*(pool.repos+1)) << " " << (*(pool.repos+1))->name << endl;
  MIL << (void*)(*(pool.repos+2)) << " " << (*(pool.repos+2))->name << endl;

  ///////////////////////////////////////////////////////////////////
  INT << "===[END]============================================" << endl << endl;
  zypp::base::LogControl::instance().logNothing();
  return 0;
}
