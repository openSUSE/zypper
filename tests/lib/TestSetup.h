#ifndef INCLUDE_TESTSETUP
#define INCLUDE_TESTSETUP
#include <iostream>

#ifndef INCLUDE_TESTSETUP_WITHOUT_BOOST
#include <boost/test/auto_unit_test.hpp>
using boost::unit_test::test_case;
#endif

#include "zypp/base/LogTools.h"
#include "zypp/ZYppFactory.h"
#include "zypp/ZYpp.h"
#include "zypp/TmpPath.h"
#include "zypp/PathInfo.h"
#include "zypp/RepoManager.h"
#include "zypp/Target.h"
#include "zypp/ResPool.h"

using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using namespace zypp;

#ifndef BOOST_CHECK_NE
#define BOOST_CHECK_NE( L, R ) BOOST_CHECK( (L) != (R) )
#endif

/** Build a test environment below a temp. root directory.
 * If a \c rootdir_r was provided to the ctor, this directory
 * will be used and it will \b not be removed.
 *
 * \note The lifetime of this objects is the lifetime of the temp. root directory.
 *
 * \code
 * #include "TestSetup.h"
 *
 * BOOST_AUTO_TEST_CASE(WhatProvides)
 * {
 *   TestSetup test( Arch_x86_64 );
 *   // test.loadTarget(); // initialize and load target
 *   test.loadRepo( TESTS_SRC_DIR"/data/openSUSE-11.1" );
 *
 *   // Here the pool is ready to be used.
 *
 * }
 * \endcode
*/
class TestSetup
{
  public:
    TestSetup( const Arch & sysarch_r = Arch() )
    { _ctor( Pathname(), sysarch_r ); }

    TestSetup( const Pathname & rootdir_r, const Arch & sysarch_r = Arch() )
    { _ctor( rootdir_r, sysarch_r ); }

    ~TestSetup()
    { USR << "DELETE TESTSETUP below " << _rootdir << endl; }

  public:
    const Pathname & root() const { return _rootdir; }

    Target &     target()      { if ( ! getZYpp()->getTarget() ) getZYpp()->initializeTarget( _rootdir ); return *getZYpp()->getTarget(); }
    RepoManager  repomanager() { return RepoManager( RepoManagerOptions::makeTestSetup( _rootdir ) ); }
    ResPool      pool()        { return ResPool::instance(); }
    ResPoolProxy poolProxy()   { return pool().proxy(); }
    sat::Pool    satpool()     { return sat::Pool::instance(); }
    Resolver &   resolver()    { return *getZYpp()->resolver(); }

  public:
    /** Load target repo. */
    void loadTarget()
    { target().load(); }
    /** Fake @System repo from url. */
    void loadTargetRepo( const Url & url_r )
    { loadRepo( url_r, sat::Pool::systemRepoAlias() ); }
    /** Fake @System repo from Path. */
    void loadTargetRepo( const Pathname & path_r )
    { loadRepo( path_r, sat::Pool::systemRepoAlias() ); }

    /** Directly load repoinfo to pool. */
    void loadRepo( RepoInfo nrepo )
    {
      RepoManager rmanager( repomanager() );
      rmanager.buildCache( nrepo );
      rmanager.loadFromCache( nrepo );
    }
    /** Directly load repo from url to pool. */
    void loadRepo( const Url & url_r, const std::string & alias_r = std::string() )
    {
      RepoInfo nrepo;
      nrepo.setAlias( alias_r.empty() ? url_r.getHost()+":"+Pathname::basename(url_r.getPathName()) : alias_r );
      nrepo.addBaseUrl( url_r );
      nrepo.setGpgCheck( false );
      loadRepo( nrepo );
    }
    /** Directly load repo from metadata(dir) or solvfile(file) to pool.
     * An empty alias is guessed.
    */
    void loadRepo( const Pathname & path_r, const std::string & alias_r = std::string() )
    {
      if ( filesystem::PathInfo( path_r ).isDir() )
      {
        loadRepo( path_r.asUrl(), alias_r );
        return;
      }
      // .solv file is loaded directly using a faked RepoInfo
      RepoInfo nrepo;
      nrepo.setAlias( alias_r.empty() ? path_r.basename() : alias_r );
      satpool().addRepoSolv( path_r, nrepo );
    }

  private:
    void _ctor( const Pathname & rootdir_r, const Arch & sysarch_r )
    {
      if ( rootdir_r.empty() )
        _rootdir = _tmprootdir.path();
      else
        filesystem::assert_dir( (_rootdir = rootdir_r) );

      if ( ! sysarch_r.empty() )
        ZConfig::instance().setSystemArchitecture( sysarch_r );
      USR << "CREATED TESTSETUP below " << _rootdir << endl;
    }
  private:
    filesystem::TmpDir _tmprootdir;
    Pathname           _rootdir;
};


#endif //INCLUDE_TESTSETUP
