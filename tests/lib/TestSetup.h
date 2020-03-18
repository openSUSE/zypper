#ifndef INCLUDE_TESTSETUP
#define INCLUDE_TESTSETUP
#include <iostream>

#ifndef INCLUDE_TESTSETUP_WITHOUT_BOOST
#include <boost/test/auto_unit_test.hpp>
using boost::unit_test::test_case;
#endif

#include <zypp/base/LogControl.h>
#include <zypp/base/LogTools.h>
#include <zypp/base/InputStream.h>
#include <zypp/base/IOStream.h>
#include <zypp/base/Flags.h>
#include <zypp/ZYppFactory.h>
#include <zypp/ZYpp.h>
#include <zypp/TmpPath.h>
#include <zypp/Glob.h>
#include <zypp/PathInfo.h>
#include <zypp/RepoManager.h>
#include <zypp/Target.h>
#include <zypp/ResPool.h>
#include <zypp/misc/LoadTestcase.h>

using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::flush;
using namespace zypp;

#ifndef BOOST_CHECK_NE
#define BOOST_CHECK_NE( L, R ) BOOST_CHECK( (L) != (R) )
#endif

#define LABELED(V) #V << ":\t" << V

enum TestSetupOptionBits
{
  TSO_CLEANROOT		= (1 <<  0),	// wipe rootdir in ctor
  TSO_REPO_DEFAULT_GPG	= (1 <<  1),	// dont turn off gpgcheck in repos
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
  struct InitLaterType {};
  static constexpr InitLaterType initLater = InitLaterType();

  TestSetup( InitLaterType )
  {}

  TestSetup( const Arch & sysarch_r = Arch_empty, const Options & options_r = Options() )
    : _pimpl { new Impl( Pathname(), sysarch_r, options_r ) }
  {}

  TestSetup( const Pathname & rootdir_r, const Arch & sysarch_r = Arch_empty, const Options & options_r = Options() )
    : _pimpl { new Impl( rootdir_r, sysarch_r, options_r ) }
  {}

  TestSetup( const Pathname & rootdir_r, const Options & options_r )
    : _pimpl { new Impl( rootdir_r, Arch_empty, options_r ) }
  {}

  void reset()
  { _pimpl.reset(); }

public:
  /** Whether directory \a path_r contains a solver testcase. */
  static bool isTestcase( const Pathname & path_r )
  {
    return  zypp::misc::testcase::LoadTestcase::None != zypp::misc::testcase::LoadTestcase::testcaseTypeAt( path_r );
  }

  /** Whether directory \a path_r contains a testsetup. */
  static bool isTestSetup( const Pathname & path_r )
  {
    return filesystem::PathInfo( path_r / "repos.d" ).isDir() && filesystem::PathInfo( path_r / "raw" ).isDir();
  }

public:
  const Pathname & root() const { return _pimpl->_rootdir; }

  Target &     target()      { if ( ! getZYpp()->getTarget() ) getZYpp()->initializeTarget( _pimpl->_rootdir ); return *getZYpp()->getTarget(); }
  RepoManager  repomanager() { return RepoManager( RepoManagerOptions::makeTestSetup( _pimpl->_rootdir ) ); }
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
      nrepo.setAlias( RepoManager::makeStupidAlias( nrepo.url() ) );
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
    if ( ! _pimpl->_options.testFlag( TSO_REPO_DEFAULT_GPG ) )
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

  /** Directly load a helix repo from some testcase.
   ** An empty alias is guessed.
   **/
  void loadHelix( const Pathname & path_r, const std::string & alias_r = std::string() )
  {
    // .solv file is loaded directly using a faked RepoInfo
    RepoInfo nrepo;
    nrepo.setAlias( alias_r.empty() ? path_r.basename() : alias_r );
    satpool().addRepoHelix( path_r, nrepo );
  }

  // Load repos included in a solver testcase.
  void loadTestcaseRepos( const Pathname & path_r, misc::testcase::LoadTestcase::TestcaseTrials * trialsP_r = nullptr )
  {
    zypp::misc::testcase::LoadTestcase loader;
    std::string err;
    if (!loader.loadTestcaseAt( path_r, &err ) ) {
      ZYPP_THROW( Exception(err) );
    }

    const auto &setup = loader.setupInfo();
    auto tempRepoManager = repomanager();
    if ( !setup.applySetup( tempRepoManager ) ) {
      ZYPP_THROW( Exception("Failed to apply setup!") );
    }

    {
      base::SetTracker<LocaleSet> localesTracker = setup.localesTracker();
      localesTracker.removed().insert( localesTracker.current().begin(), localesTracker.current().end() );
      satpool().initRequestedLocales( localesTracker.removed() );

      localesTracker.added().insert( localesTracker.current().begin(), localesTracker.current().end() );
      satpool().setRequestedLocales( localesTracker.added() );
    }
    poolProxy(); // prepare
    if ( trialsP_r )
      *trialsP_r = loader.trialInfo();
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

public:
  /** Detect and load the system located at \a sysRoot.
     *
     * \a sysRoot needs to be a directory containing either a SolverTestcase,
     * a TestSetup system or a real system. The  provided repostitories are
     * loaded into the pool (without refresh).
    */
  static void LoadSystemAt( const Pathname & sysRoot, const Arch & _testSetupArch_r = Arch_x86_64 )
  {
    if ( ! PathInfo( sysRoot ).isDir() )
      ZYPP_THROW( Exception("sysRoot argument needs to be a directory") );

    if ( TestSetup::isTestcase( sysRoot ) )
    {
      USR << str::form( "*** Load Testcase from '%s'", sysRoot.c_str() ) << endl;
      TestSetup test;
      test.loadTestcaseRepos( sysRoot );
    }
    else if ( TestSetup::isTestSetup( sysRoot ) )
    {
      USR << str::form( "*** Load TestSetup from '%s'", sysRoot.c_str() ) << endl;

      TestSetup test( sysRoot, _testSetupArch_r );
      test.loadRepos();

      Pathname solvCachePath( RepoManagerOptions::makeTestSetup( test.root() ).repoSolvCachePath );
      Pathname fakeTargetSolv( solvCachePath / sat::Pool::systemRepoAlias() / "solv" );
      if ( PathInfo( fakeTargetSolv ).isFile() )
      {
        USR << str::form( "*** Fake TestSetup Target from '%s'", fakeTargetSolv.c_str() ) << endl;
        test.target();
        test.loadTargetRepo( fakeTargetSolv );
      }
    }
    else
    {
      sat::Pool satpool( sat::Pool::instance() );
      // a system
      USR << str::form( "*** Load system at '%s'", sysRoot.c_str() ) << endl;
      if ( 1 )
      {
        USR << "*** load target '" << Repository::systemRepoAlias() << "'\t" << endl;
        getZYpp()->initializeTarget( sysRoot );
        getZYpp()->target()->load();
        USR << satpool.systemRepo() << endl;
      }

      if ( 1 )
      {
        RepoManager repoManager( sysRoot );
        RepoInfoList repos = repoManager.knownRepositories();
        for_( it, repos.begin(), repos.end() )
        {
          RepoInfo & nrepo( *it );

          if ( ! nrepo.enabled() )
            continue;

          if ( ! repoManager.isCached( nrepo ) )
          {
            USR << str::form( "*** omit uncached repo '%s' (do 'zypper refresh')", nrepo.name().c_str() ) << endl;
            continue;
          }

          USR << str::form( "*** load repo '%s'\t", nrepo.name().c_str() ) << flush;
          try
          {
            repoManager.loadFromCache( nrepo );
            USR << satpool.reposFind( nrepo.alias() ) << endl;
          }
          catch ( const Exception & exp )
          {
            USR << exp.asString() + "\n" + exp.historyAsString() << endl;
            USR << str::form( "*** omit broken repo '%s' (do 'zypper refresh')", nrepo.name().c_str() ) << endl;
            continue;
          }
        }
      }
    }
  }

private:
  struct Impl
  {
    Impl( const Pathname & rootdir_r, const Arch & sysarch_r, const Options & options_r )
    {
      _options = options_r;

      if ( rootdir_r.empty() )
        _rootdir = _tmprootdir.path();
      else
      {
        filesystem::assert_dir( (_rootdir = rootdir_r) );
        if ( _options.testFlag( TSO_CLEANROOT ) )
          filesystem::clean_dir( _rootdir );
      }

      // erase any old pool content...
      getZYpp()->finishTarget();
      sat::Pool::instance().reposEraseAll();
      // prepare for the new one...
      ZConfig::instance().setRepoManagerRoot( _rootdir );

      if ( ! sysarch_r.empty() )
        ZConfig::instance().setSystemArchitecture( sysarch_r );
      USR << "CREATED TESTSETUP below " << _rootdir << endl;
    }

    ~Impl()
    { USR << (_tmprootdir.path() == _rootdir ? "DELETE" : "KEEP") << " TESTSETUP below " << _rootdir << endl; }

    filesystem::TmpDir _tmprootdir;
    Pathname           _rootdir;
    Options            _options;
  };

  std::unique_ptr<Impl> _pimpl;	// maybe worth creating RW_pointer traits for it
};

#endif //INCLUDE_TESTSETUP
