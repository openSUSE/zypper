#ifndef INCLUDE_TESTSETUP
#define INCLUDE_TESTSETUP
#include <iostream>

#ifndef INCLUDE_TESTSETUP_WITHOUT_BOOST
#include <boost/test/auto_unit_test.hpp>
using boost::unit_test::test_case;
#endif

#include "zypp/base/LogControl.h"
#include "zypp/base/LogTools.h"
#include "zypp/base/Flags.h"
#include "zypp/ZYppFactory.h"
#include "zypp/ZYpp.h"
#include "zypp/TmpPath.h"
#include "zypp/Glob.h"
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

enum TestSetupOptionBits
{
  TSO_CLEANROOT = (1 <<  0)
};
ZYPP_DECLARE_FLAGS_AND_OPERATORS( TestSetupOptions, TestSetupOptionBits );

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
 *   // enabls loging fot the scope of this block:
 *   // base::LogControl::TmpLineWriter shutUp( new log::FileLineWriter( "/tmp/YLOG" ) );
 *
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
    typedef TestSetupOptions Options;

  public:
    TestSetup( const Arch & sysarch_r = Arch() )
    { _ctor( Pathname(), sysarch_r, Options() ); }

    TestSetup( const Pathname & rootdir_r, const Arch & sysarch_r = Arch(), const Options & options_r = Options() )
    { _ctor( rootdir_r, sysarch_r, options_r ); }

    TestSetup( const Pathname & rootdir_r, const Options & options_r )
    { _ctor( rootdir_r, Arch(), options_r ); }

    ~TestSetup()
    { USR << (_tmprootdir.path() == _rootdir ? "DELETE" : "KEEP") << " TESTSETUP below " << _rootdir << endl; }

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
    /** Fake @System repo from helix repo. */
    void loadTargetHelix( const Pathname & path_r )
    { loadHelix( path_r, sat::Pool::systemRepoAlias() ); }

  public:
    /** Directly load repoinfo to pool. */
    void loadRepo( RepoInfo nrepo )
    {
      RepoManager rmanager( repomanager() );
      if ( rmanager.hasRepo( nrepo ) )
        rmanager.modifyRepository( nrepo );
      else
        rmanager.addRepository( nrepo );
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
    /** Directly load repo from some location (url or absolute(!)path).
     * An empty alias is guessed.
    */
    void loadRepo( const std::string & loc_r, const std::string & alias_r = std::string() )
    {
      if ( *loc_r.c_str() == '/' )
      {
        loadRepo( Pathname( loc_r ), alias_r );
      }
      else
      {
        loadRepo( Url( loc_r ), alias_r );
      }
    }
    /** Directly load repo from some location (url or absolute(!)path).
     * An empty alias is guessed.
    */
    void loadRepo( const char * loc_r, const std::string & alias_r = std::string() )
    { loadRepo( std::string( loc_r ? loc_r : "" ), alias_r ); }

  public:
    /** Directly load a helix repo from some testcase.
     * An empty alias is guessed.
    */
    void loadHelix( const Pathname & path_r, const std::string & alias_r = std::string() )
    {
      // .solv file is loaded directly using a faked RepoInfo
      RepoInfo nrepo;
      nrepo.setAlias( alias_r.empty() ? path_r.basename() : alias_r );
      satpool().addRepoHelix( path_r, nrepo );
    }

    // Load repos included in a solver testcase.
    void loadTestcaseRepos( const Pathname & path_r )
    {
      if ( ! filesystem::PathInfo( path_r ).isDir() )
      {
        ERR << "No dir " << filesystem::PathInfo( path_r ) << endl;
        return;
      }
      filesystem::Glob files( path_r/"*{.xml,.xml.gz}", filesystem::Glob::_BRACE );
      for_( it, files.begin(), files.end() )
      {
        std::string basename( Pathname::basename( *it ) );
        if ( str::hasPrefix( basename, "solver-test.xml" ) )
          continue; // master index currently unevaluated
        if ( str::hasPrefix( basename, "solver-system.xml" ) )
          loadTargetHelix( *it );
        else
          loadHelix( *it );
      }
    }

  public:
    /** Load all enabled repos in repos.d to pool. */
    void loadRepos()
    {
      RepoManager repoManager( repomanager() );
      RepoInfoList repos = repoManager.knownRepositories();
      for ( RepoInfoList::iterator it = repos.begin(); it != repos.end(); ++it )
      {
        RepoInfo & nrepo( *it );
        USR << nrepo << endl;

        if ( ! nrepo.enabled() )
          continue;

        if ( ! repoManager.isCached( nrepo ) || nrepo.type() == repo::RepoType::RPMPLAINDIR )
        {
          if ( repoManager.isCached( nrepo ) )
          {
            USR << "cleanCache" << endl;
            repoManager.cleanCache( nrepo );
          }
          //USR << "refreshMetadata" << endl;
          //repoManager.refreshMetadata( nrepo );
          USR << "buildCache" << endl;
          repoManager.buildCache( nrepo );
        }
        USR << "Create from cache" << endl;
        repoManager.loadFromCache( nrepo );
      }
    }

  private:
    void _ctor( const Pathname & rootdir_r, const Arch & sysarch_r, const Options & options_r )
    {
      if ( rootdir_r.empty() )
        _rootdir = _tmprootdir.path();
      else
      {
        filesystem::assert_dir( (_rootdir = rootdir_r) );
        if ( options_r.testFlag( TSO_CLEANROOT ) )
          filesystem::clean_dir( _rootdir );
      }

      if ( ! sysarch_r.empty() )
        ZConfig::instance().setSystemArchitecture( sysarch_r );
      USR << "CREATED TESTSETUP below " << _rootdir << endl;
    }
  private:
    filesystem::TmpDir _tmprootdir;
    Pathname           _rootdir;
};


#endif //INCLUDE_TESTSETUP
