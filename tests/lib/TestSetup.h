#ifndef INCLUDE_TESTSETUP
#define INCLUDE_TESTSETUP
#include <iostream>

#ifndef INCLUDE_TESTSETUP_WITHOUT_BOOST
#include <boost/test/auto_unit_test.hpp>
using boost::unit_test::test_case;
#endif

#include "zypp/base/LogControl.h"
#include "zypp/base/LogTools.h"
#include "zypp/base/InputStream.h"
#include "zypp/base/IOStream.h"
#include "zypp/base/Flags.h"
#include "zypp/ZYppFactory.h"
#include "zypp/ZYpp.h"
#include "zypp/TmpPath.h"
#include "zypp/Glob.h"
#include "zypp/PathInfo.h"
#include "zypp/RepoManager.h"
#include "zypp/Target.h"
#include "zypp/ResPool.h"

#include "Zypper.h"
#include "output/OutNormal.h"

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

inline std::string getXmlNodeVal( const std::string & line_r, const std::string & node_r )
{
  std::string::size_type pos = line_r.find( node_r + "=\"" );
  if ( pos != std::string::npos )
  {
    pos += node_r.size() + 2;
    std::string::size_type epos = line_r.find( "\"", pos );
    return line_r.substr( pos, epos-pos );
  }
  return std::string();
}

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
 *   // enables logging fot the scope of this block:
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
    TestSetup( const Arch & sysarch_r = Arch_empty )
    { _ctor( Pathname(), sysarch_r, Options() ); }

    TestSetup( const Pathname & rootdir_r, const Arch & sysarch_r = Arch_empty, const Options & options_r = Options() )
    { _ctor( rootdir_r, sysarch_r, options_r ); }

    TestSetup( const Pathname & rootdir_r, const Options & options_r )
    { _ctor( rootdir_r, Arch_empty, options_r ); }

    ~TestSetup()
    { USR << (_tmprootdir.path() == _rootdir ? "DELETE" : "KEEP") << " TESTSETUP below " << _rootdir << endl; }

  public:
    /** Whether directory \a path_r contains a solver testcase. */
    static bool isTestcase( const Pathname & path_r )
    {
      return filesystem::PathInfo( path_r / "solver-test.xml" ).isFile();
    }

    /** Whether directory \a path_r contains a testsetup. */
    static bool isTestSetup( const Pathname & path_r )
    {
      return filesystem::PathInfo( path_r / "repos.d" ).isDir() && filesystem::PathInfo( path_r / "raw" ).isDir();
    }

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

  private:
    // repo data from solver-test.xml
    struct RepoD {
      DefaultIntegral<unsigned,0> priority;
      std::string alias;
      Url url;
    };

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
      filesystem::PathInfo pi( path_r / "solver-test.xml" );
      if ( ! pi.isFile() )
      {
        ERR << "No testcase in " << filesystem::PathInfo( path_r ) << endl;
        return;
      }
      // dumb parse
      InputStream infile( pi.path() );
      Arch sysarch( Arch_empty );
      Url guessedUrl;
      typedef std::map<std::string,RepoD> RepoI;
      RepoI repoi;
      for( iostr::EachLine in( infile ); in; in.next() )
      {
        if ( str::hasPrefix( *in, "\t<channel" ) )
        {
          RepoD & repod( repoi[getXmlNodeVal( *in, "file" )] );

          repod.alias = getXmlNodeVal( *in, "name" );
          repod.priority = str::strtonum<unsigned>( getXmlNodeVal( *in, "priority" ) );
          repod.url = guessedUrl;
          guessedUrl = Url();
        }
        else if ( str::hasPrefix( *in, "\t- url " ) )
        {
          std::string::size_type pos = in->find( ": " );
          if ( pos != std::string::npos )
          {
            guessedUrl = Url( in->substr( pos+2 ) );
          }
        }
        else if ( str::hasPrefix( *in, "\t<locale" ) )
        {
          satpool().addRequestedLocale( Locale( getXmlNodeVal( *in, "name" ) ) );
        }
       else if ( sysarch.empty() && str::hasPrefix( *in, "<setup" ) )
        {
          sysarch = Arch( getXmlNodeVal( *in, "arch" ) );
          if ( ! sysarch.empty() )
            ZConfig::instance().setSystemArchitecture( sysarch );
        }
      }

      //
      filesystem::Glob files( path_r/"*{.xml,.xml.gz}", filesystem::Glob::_BRACE );
      for_( it, files.begin(), files.end() )
      {
        std::string basename( Pathname::basename( *it ) );
        if ( str::hasPrefix( basename, "solver-test.xml" ) )
          continue; // master index currently unevaluated
        if ( str::hasPrefix( basename, "solver-system.xml" ) )
          loadTargetHelix( *it );
        else
        {
          const RepoD & repod( repoi[basename] );

          RepoInfo nrepo;
          nrepo.setAlias( repod.alias.empty() ? basename : repod.alias );
          nrepo.setPriority( repod.priority );
          nrepo.setBaseUrl( repod.url );
          satpool().addRepoHelix( *it, nrepo );
        }
      }

      poolProxy(); // prepare
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

      // set up the Zypper instance

      Zypper & zypper = *Zypper::instance();
      zypper.globalOptsNoConst().root_dir = _rootdir.asString();
      zypper.globalOptsNoConst().rm_options = zypp::RepoManagerOptions(_rootdir.asString());
      zypper.globalOptsNoConst().rm_options.knownReposPath = _rootdir / "repos.d";
      zypper.setOutputWriter(new OutNormal(Out::DEBUG));

      if ( ! sysarch_r.empty() )
        ZConfig::instance().setSystemArchitecture( sysarch_r );
      USR << "CREATED TESTSETUP below " << _rootdir << endl;
    }
  private:
    filesystem::TmpDir _tmprootdir;
    Pathname           _rootdir;
};


#endif //INCLUDE_TESTSETUP
