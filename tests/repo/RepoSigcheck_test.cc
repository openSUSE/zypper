#include <iostream>

#include <boost/test/auto_unit_test.hpp>

#include "zypp/ZYppFactory.h"
#include "zypp/RepoManager.h"
#include "TestSetup.h"

using namespace boost::unit_test;
using namespace zypp;
using std::cout;
using std::endl;

#define TC_VERBOSE 0

#define COUT if ( TC_VERBOSE ) std::cout
#define TAG COUT << "*** " << __PRETTY_FUNCTION__ << endl

TestSetup test( Arch_x86_64, TSO_REPO_DEFAULT_GPG );
const Pathname DATADIR( TESTS_SRC_DIR "/repo/RepoSigcheck" );

///////////////////////////////////////////////////////////////////

struct KeyRingReceiver : public callback::ReceiveReport<KeyRingReport>
{
  typedef callback::ReceiveReport<KeyRingReport> Base;

  KeyRingReceiver()		{ TAG; connect(); }
  ~KeyRingReceiver()		{ TAG; }

  virtual void reportbegin()	{ TAG; cblistCheck( __FUNCTION__ ); }
  virtual void reportend()	{ TAG; cblistCheck( __FUNCTION__ ); }

  virtual KeyTrust askUserToAcceptKey( const PublicKey &key, const KeyContext &keycontext = KeyContext() )
  {
    TAG; cblistCheck( __FUNCTION__ );
    return Base::askUserToAcceptKey( key , keycontext );
  }

  virtual void infoVerify( const std::string & file_r, const PublicKeyData & keyData_r, const KeyContext &keycontext = KeyContext() )
  {
    TAG; cblistCheck( __FUNCTION__ );
    return Base::infoVerify( file_r, keyData_r, keycontext );
  }

  virtual bool askUserToAcceptUnsignedFile( const std::string &file, const KeyContext &keycontext = KeyContext() )
  {
    TAG; cblistCheck( __FUNCTION__ );
    return Base::askUserToAcceptUnsignedFile( file, keycontext );
  }

  virtual bool askUserToAcceptUnknownKey( const std::string &file, const std::string &id, const KeyContext &keycontext = KeyContext() )
  {
    TAG;  cblistCheck( __FUNCTION__ );
    return Base::askUserToAcceptUnknownKey( file, id, keycontext );
  }

  virtual bool askUserToAcceptVerificationFailed( const std::string &file, const PublicKey &key, const KeyContext &keycontext = KeyContext() )
  {
    TAG;  cblistCheck( __FUNCTION__ );
    return Base::askUserToAcceptVerificationFailed( file, key, keycontext );
  }

public:
  typedef std::list<std::string> CallbackList;

  void cblistCheck( const std::string & cb_r )
  {
    BOOST_CHECK_EQUAL( _cblist.empty(), false );
    if ( !_cblist.empty() )
    {
      BOOST_CHECK_EQUAL( _cblist.front(), cb_r );
      _cblist.pop_front();
    }
  }

  CallbackList _cblist;	// expected callback sequence list

} krCallback;

inline std::string chr( const bool & v )
{ return v ? "1" : "0"; }

inline std::string chr( const TriBool & v )
{
  return indeterminate(v) ? "*" : chr( bool(v) );
}

///////////////////////////////////////////////////////////////////
/*
 * Check the RepoInfo methods returning which checks to perform
 * based on the global (zypp.conf) and local (.repo file) settings.
 *
 * *** See table in RepoInfo.h:'Repository gpgchecks'
 */
BOOST_AUTO_TEST_CASE(init)
{
  ZConfig & zcfg( ZConfig::instance() );

  RepoInfo repo;
  std::initializer_list<TriBool> tribools( { TriBool(indeterminate), TriBool(true), TriBool(false) } );

  // global zconfig values...
  for ( bool g_GpgCheck : { true, false } )
  {
    zcfg.setGpgCheck( g_GpgCheck );
    for ( TriBool g_RepoGpgCheck : tribools )
    {
      zcfg.setRepoGpgCheck( g_RepoGpgCheck );
      for ( TriBool g_PkgGpgCheck : tribools )
      {
	zcfg.setPkgGpgCheck( g_PkgGpgCheck );

	// .repo values
	for ( TriBool r_GpgCheck : tribools )
	{
	  repo.setGpgCheck( r_GpgCheck );
	  for ( TriBool r_RepoGpgCheck : tribools )
	  {
	    repo.setRepoGpgCheck( r_RepoGpgCheck );
	    for ( TriBool r_PkgGpgCheck : tribools )
	    {
	      repo.setPkgGpgCheck( r_PkgGpgCheck );

	      // check the repo methods returning what to do:
	      bool	cfgGpgCheck	= indeterminate(r_GpgCheck)                                  ? g_GpgCheck     : bool(r_GpgCheck);
	      TriBool	cfgRepoGpgCheck	= indeterminate(r_GpgCheck) && indeterminate(r_RepoGpgCheck) ? g_RepoGpgCheck : r_RepoGpgCheck;
	      TriBool	cfgPkgGpgCheck	= indeterminate(r_GpgCheck) && indeterminate(r_PkgGpgCheck)  ? g_PkgGpgCheck  : r_PkgGpgCheck;
#if ( TC_VERBOSE )
	      COUT << chr(cfgGpgCheck) << "\t" << chr(cfgRepoGpgCheck) << "\t" << chr(cfgPkgGpgCheck)
		   << "\t(" << chr(r_GpgCheck)     << "," << chr(g_GpgCheck)     << ")"
		   << "\t(" << chr(r_RepoGpgCheck) << "," << chr(g_RepoGpgCheck) << ")"
		   << "\t(" << chr(r_PkgGpgCheck)  << "," << chr(g_PkgGpgCheck)  << ")"
	           << flush;
#endif

	      // default gpgCeck follows config
	      BOOST_CHECK_EQUAL( repo.gpgCheck(), cfgGpgCheck );


	      // repoGpgCheck follows gpgCeck
	      // explicitly defined it alters mandatory check
	      bool willCheckRepo  = repo.repoGpgCheck();
	      bool mandatoryCheck = repo.repoGpgCheckIsMandatory();
#if ( TC_VERBOSE )
	      COUT << "\t" << ( willCheckRepo ? ( mandatoryCheck ? "!" : "+" ) : "-" ) << flush;
#endif
	      if ( mandatoryCheck )	// be a subset of willCheckRepo!
		BOOST_CHECK_EQUAL( willCheckRepo, mandatoryCheck );

	      if ( cfgGpgCheck )
	      {
		BOOST_CHECK_EQUAL( willCheckRepo,  true );
		BOOST_CHECK_EQUAL( mandatoryCheck, !bool(!cfgRepoGpgCheck) );	// TriBool: !false = true or indeterminate
	      }
	      else
	      {
		BOOST_CHECK_EQUAL( willCheckRepo,  bool(cfgRepoGpgCheck) );
		BOOST_CHECK_EQUAL( mandatoryCheck, bool(cfgRepoGpgCheck) );
	      }


	      // pkgGpgCheck may depend on the repoGpgCheck result
	      for ( TriBool r_validSignature : tribools )	// indeterminate <==> unsigned repo
	      {
		repo.setValidRepoSignature( r_validSignature );

		if ( r_validSignature && !willCheckRepo )
		  // RepoInfo must invalidate any valid (old) signature as soon as the repo check
		  // is turned off. This prevents showing 'valid sig' for not yet refreshed repos.
		  // Instead show 'won't be checked' immediately.
		  BOOST_CHECK( bool(!repo.validRepoSignature()) );
		else
		  BOOST_CHECK( sameTriboolState( repo.validRepoSignature(), r_validSignature ) );

		bool willCheckPkg   = repo.pkgGpgCheck();
		bool mandatoryCheck = repo.pkgGpgCheckIsMandatory();
#if ( TC_VERBOSE )
		COUT << "\t" << chr(r_validSignature) << ( willCheckPkg ? ( mandatoryCheck ? "!" : "+" ) : "-" ) << flush;
#endif
		if ( mandatoryCheck )	// be a subset of willCheckPkg!
		  BOOST_CHECK_EQUAL( willCheckPkg, mandatoryCheck );

		if ( cfgPkgGpgCheck )
		{
		  BOOST_CHECK_EQUAL( willCheckPkg,   true );
		  BOOST_CHECK_EQUAL( mandatoryCheck, true );
		}
		else if ( cfgGpgCheck )
		{
		  if ( r_validSignature )
		  {
		    BOOST_CHECK_EQUAL( willCheckPkg,   false );
		    BOOST_CHECK_EQUAL( mandatoryCheck, false );
		  }
		  else // TriBool: !true = false or indeterminate/unsigned
		  {
		    BOOST_CHECK_EQUAL( willCheckPkg,   true );
		    BOOST_CHECK_EQUAL( mandatoryCheck, !bool(!cfgPkgGpgCheck) ); // TriBool: !false = true or indeterminate/unsigned
		  }
		}
		else
		{
		  BOOST_CHECK_EQUAL( willCheckPkg,   false );
		  BOOST_CHECK_EQUAL( mandatoryCheck, false );
		}
	      }
#if ( TC_VERBOSE )
	      COUT << endl;
#endif

	    }
	  }
	}
      }
    }
  }
  // reset to defaults:
  zcfg.setGpgCheck	( true );
  zcfg.setRepoGpgCheck	( indeterminate );
  zcfg.setPkgGpgCheck	( indeterminate );
}

// RAII: Protect ZConfig value changes from escaping the block scope
struct ZConfigGuard
{
  ZConfigGuard()
  : _zcfg( ZConfig::instance() )
  {
    _g = _zcfg.gpgCheck();
    _r = _zcfg.repoGpgCheck();
    _p = _zcfg.pkgGpgCheck();
  }

  ~ZConfigGuard()
  {
    _zcfg.setGpgCheck    ( _g );
    _zcfg.setRepoGpgCheck( _r );
    _zcfg.setPkgGpgCheck ( _p );
  }

  ZConfig * operator->() { return &_zcfg; }

  ZConfig & 	_zcfg;
  bool		_g;
  TriBool	_r;
  TriBool	_p;
};


// RAII: Set and reset KeyRingReceiver callback list and response bits for new testcase
struct KeyRingGuard
{
  KeyRingGuard ( KeyRing::DefaultAccept accept_r = KeyRing::ACCEPT_NOTHING )
  {
    KeyRing::setDefaultAccept( accept_r );
    krCallback._cblist.clear();
#if ( TC_VERBOSE )
    COUT << "================================================================================" << endl;
    KeyRing & keyRing( *getZYpp()->keyRing() );
    COUT << "K " << keyRing.publicKeys() << endl;
    COUT << "T " << keyRing.trustedPublicKeys() << endl;
    COUT << KeyRing::defaultAccept() << endl;

    ZConfig & zcfg( ZConfig::instance() );
    COUT << "ZConf " << chr( zcfg.gpgCheck() ) << chr( zcfg.repoGpgCheck() ) << chr( zcfg.pkgGpgCheck() ) << endl;
#endif
  }

  ~KeyRingGuard()
  {
    BOOST_CHECK_EQUAL( krCallback._cblist.empty(), true );
    KeyRing::setDefaultAccept( KeyRing::ACCEPT_NOTHING );
    krCallback._cblist.clear();
  }
};

void testLoadRepo( bool succeed_r, 				// whether loadRepos should succeed or fail with RepoException
		   const std::string & repo_r,			// name of the test repo to load
		   KeyRing::DefaultAccept accept_r,		// Callback response bits to set (mimics user input)
		   KeyRingReceiver::CallbackList cblist_r )	// Callback sequence list expected
{
  KeyRingGuard _guard( accept_r );
  krCallback._cblist = std::move(cblist_r);
  if ( succeed_r )
    test.loadRepo( DATADIR/repo_r, repo_r );
  else
    BOOST_CHECK_THROW( test.loadRepo( DATADIR/repo_r, repo_r ), repo::RepoException );
}

// ACCEPT_NOTHING             = 0x0000,
// ACCEPT_UNSIGNED_FILE       = 0x0001,
// ACCEPT_UNKNOWNKEY          = 0x0002,
// TRUST_KEY_TEMPORARILY      = 0x0004,
// TRUST_AND_IMPORT_KEY       = 0x0008,
// ACCEPT_VERIFICATION_FAILED = 0x0010,

BOOST_AUTO_TEST_CASE(unsigned_repo)
{
  // askUserToAcceptUnsignedFile actually depends on the gpgcheck settings.
  // Mandatory on 'R' cases. Otherwise an unsigend repo is accepted but 'pkggpg on'
  //  is enforced.
  ZConfigGuard zcfg;
  zcfg->setRepoGpgCheck( false );	// unsafe

  std::string repo( "unsigned_repo" );
  testLoadRepo( true, repo, KeyRing::ACCEPT_NOTHING,
		{ } );

  zcfg->setRepoGpgCheck( indeterminate );	// the default

  testLoadRepo( false, repo, KeyRing::ACCEPT_NOTHING,
		{ "reportbegin", "askUserToAcceptUnsignedFile", "reportend" } );
  testLoadRepo( true, repo, KeyRing::ACCEPT_UNSIGNED_FILE,
		{ "reportbegin", "askUserToAcceptUnsignedFile", "reportend" } );
}

BOOST_AUTO_TEST_CASE(unknownkey_repo)
{
  std::string repo( "unknownkey_repo" );
  testLoadRepo( false, repo, KeyRing::ACCEPT_NOTHING,
		{ "reportbegin", "askUserToAcceptUnknownKey", "reportend" } );
  testLoadRepo( true, repo, KeyRing::ACCEPT_UNKNOWNKEY,
		{ "reportbegin", "askUserToAcceptUnknownKey", "reportend" } );
}


BOOST_AUTO_TEST_CASE(wrongsig_repo)
{
  std::string repo( "wrongsig_repo" );
  // IMPORTED KEYS WILL STAY IN KEYRING! FIXIT if it disturbs subsequent tests

  // 1st testcase with a key, so on the fly check askUserToAcceptKey
  // being called unless the key is imported in the trusted ring
  testLoadRepo( false, repo, KeyRing::ACCEPT_NOTHING,
		{ "reportbegin", "askUserToAcceptKey", "reportend" } );
  testLoadRepo( false, repo, KeyRing::TRUST_KEY_TEMPORARILY,
		{ "reportbegin", "askUserToAcceptKey", "infoVerify", "askUserToAcceptVerificationFailed", "reportend" } );
  testLoadRepo( false, repo, KeyRing::ACCEPT_NOTHING,
		{ "reportbegin", "askUserToAcceptKey", "reportend" } );
  testLoadRepo( false, repo, KeyRing::TRUST_AND_IMPORT_KEY,
		{ "reportbegin", "askUserToAcceptKey", "infoVerify", "askUserToAcceptVerificationFailed", "reportend" } );

  // Now the key is in the trusted ring (no more askUserToAcceptKey)
  testLoadRepo( false, repo, KeyRing::ACCEPT_NOTHING,
		{ "reportbegin", "infoVerify", "askUserToAcceptVerificationFailed", "reportend" } );
  testLoadRepo( true, repo, KeyRing::KeyRing::ACCEPT_VERIFICATION_FAILED,
		{ "reportbegin", "infoVerify", "askUserToAcceptVerificationFailed", "reportend" } );
}

BOOST_AUTO_TEST_CASE(signed_repo)
{
  std::string repo( "signed_repo" );
  testLoadRepo( true, repo, KeyRing::KeyRing::ACCEPT_NOTHING,	// relies on wrongsig_repo having accepted the key! (already in trusted ring)
		{ "reportbegin", "infoVerify", "reportend" } );
}


BOOST_AUTO_TEST_CASE(summary)
{
  KeyRingGuard _guard;
  KeyRing & keyRing( *getZYpp()->keyRing() );
  BOOST_CHECK_EQUAL( keyRing.publicKeys().size(),		1 );
  BOOST_CHECK_EQUAL( keyRing.trustedPublicKeys().size(),	1 );
  BOOST_CHECK_EQUAL( KeyRing::defaultAccept(),			KeyRing::ACCEPT_NOTHING );
  BOOST_CHECK_EQUAL( test.satpool().repos().size(),		5 );	//
}
