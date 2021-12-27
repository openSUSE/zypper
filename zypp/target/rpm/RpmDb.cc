/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/target/rpm/RpmDb.cc
 *
*/
#include "librpm.h"
extern "C"
{
#include <rpm/rpmcli.h>
#include <rpm/rpmlog.h>
}
#include <cstdlib>
#include <cstdio>
#include <ctime>

#include <iostream>
#include <fstream>
#include <sstream>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <algorithm>

#include <zypp-core/base/StringV.h>
#include <zypp/base/Logger.h>
#include <zypp/base/String.h>
#include <zypp/base/Gettext.h>
#include <zypp/base/LocaleGuard.h>
#include <zypp-core/base/DtorReset>

#include <zypp/Date.h>
#include <zypp/Pathname.h>
#include <zypp/PathInfo.h>
#include <zypp/PublicKey.h>
#include <zypp-core/ui/ProgressData>

#include <zypp/target/rpm/RpmDb.h>
#include <zypp/target/rpm/RpmCallbacks.h>

#include <zypp/HistoryLog.h>
#include <zypp/target/rpm/librpmDb.h>
#include <zypp/target/rpm/RpmException.h>
#include <zypp/TmpPath.h>
#include <zypp/KeyRing.h>
#include <zypp/KeyManager.h>
#include <zypp/ZYppFactory.h>
#include <zypp/ZConfig.h>
#include <zypp/base/IOTools.h>

using std::endl;
using namespace zypp::filesystem;

#define WARNINGMAILPATH		"/var/log/YaST2/"
#define FILEFORBACKUPFILES	"YaSTBackupModifiedFiles"
#define MAXRPMMESSAGELINES	10000

#define WORKAROUNDRPMPWDBUG

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "librpmDb"

namespace zypp
{
  namespace zypp_readonly_hack
  {
    bool IGotIt(); // in readonly-mode
  }
  namespace env
  {
    inline bool ZYPP_RPM_DEBUG()
    {
      static bool val = [](){
        const char * env = getenv("ZYPP_RPM_DEBUG");
        return( env && str::strToBool( env, true ) );
      }();
      return val;
    }
  } // namespace env
namespace target
{
namespace rpm
{
  const callback::UserData::ContentType InstallResolvableReport::contentRpmout( "rpmout","installpkg" );
  const callback::UserData::ContentType RemoveResolvableReport::contentRpmout( "rpmout","removepkg" );

namespace
{
#if 1 // No more need to escape whitespace since rpm-4.4.2.3
const char* quoteInFilename_m = "\'\"";
#else
const char* quoteInFilename_m = " \t\'\"";
#endif
inline std::string rpmQuoteFilename( const Pathname & path_r )
{
  std::string path( path_r.asString() );
  for ( std::string::size_type pos = path.find_first_of( quoteInFilename_m );
        pos != std::string::npos;
        pos = path.find_first_of( quoteInFilename_m, pos ) )
  {
    path.insert( pos, "\\" );
    pos += 2; // skip '\\' and the quoted char.
  }
  return path;
}


  /** Workaround bnc#827609 - rpm needs a readable pwd so we
   * chdir to /. Turn realtive pathnames into absolute ones
   * by prepending cwd so rpm still finds them
   */
  inline Pathname workaroundRpmPwdBug( Pathname path_r )
  {
#if defined(WORKAROUNDRPMPWDBUG)
    if ( path_r.relative() )
    {
      // try to prepend cwd
      AutoDispose<char*> cwd( ::get_current_dir_name(), ::free );
      if ( cwd )
        return Pathname( cwd ) / path_r;
      WAR << "Can't get cwd!" << endl;
    }
#endif
    return path_r;	// no problem with absolute pathnames
  }
}

struct KeyRingSignalReceiver : callback::ReceiveReport<KeyRingSignals>
{
  KeyRingSignalReceiver(RpmDb &rpmdb) : _rpmdb(rpmdb)
  {
    connect();
  }

  ~KeyRingSignalReceiver()
  {
    disconnect();
  }

  virtual void trustedKeyAdded( const PublicKey &key )
  {
    MIL << "trusted key added to zypp Keyring. Importing..." << endl;
    _rpmdb.importPubkey( key );
  }

  virtual void trustedKeyRemoved( const PublicKey &key  )
  {
    MIL << "Trusted key removed from zypp Keyring. Removing..." << endl;
    _rpmdb.removePubkey( key );
  }

  RpmDb &_rpmdb;
};

static shared_ptr<KeyRingSignalReceiver> sKeyRingReceiver;

unsigned diffFiles(const std::string file1, const std::string file2, std::string& out, int maxlines)
{
  const char* argv[] =
    {
      "diff",
      "-u",
      file1.c_str(),
      file2.c_str(),
      NULL
    };
  ExternalProgram prog(argv,ExternalProgram::Discard_Stderr, false, -1, true);

  //if(!prog)
  //return 2;

  std::string line;
  int count = 0;
  for (line = prog.receiveLine(), count=0;
       !line.empty();
       line = prog.receiveLine(), count++ )
  {
    if (maxlines<0?true:count<maxlines)
      out+=line;
  }

  return prog.close();
}



/******************************************************************
 **
 **
 **	FUNCTION NAME : stringPath
 **	FUNCTION TYPE : inline std::string
*/
inline std::string stringPath( const Pathname & root_r, const Pathname & sub_r )
{
  return librpmDb::stringPath( root_r, sub_r );
}

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : RpmDb
//
///////////////////////////////////////////////////////////////////

#define FAILIFNOTINITIALIZED if( ! initialized() ) { ZYPP_THROW(RpmDbNotOpenException()); }

///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::RpmDb
//	METHOD TYPE : Constructor
//
RpmDb::RpmDb()
    : _backuppath ("/var/adm/backup")
    , _packagebackups(false)
{
  process = 0;
  exit_code = -1;
  librpmDb::globalInit();
  // Some rpm versions are patched not to abort installation if
  // symlink creation failed.
  setenv( "RPM_IgnoreFailedSymlinks", "1", 1 );
  sKeyRingReceiver.reset(new KeyRingSignalReceiver(*this));
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::~RpmDb
//	METHOD TYPE : Destructor
//
RpmDb::~RpmDb()
{
  MIL << "~RpmDb()" << endl;
  closeDatabase();
  delete process;
  MIL  << "~RpmDb() end" << endl;
  sKeyRingReceiver.reset();
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::dumpOn
//	METHOD TYPE : std::ostream &
//
std::ostream & RpmDb::dumpOn( std::ostream & str ) const
{
  return str << "RpmDb[" << stringPath( _root, _dbPath ) << "]";
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::initDatabase
//	METHOD TYPE : PMError
//
void RpmDb::initDatabase( Pathname root_r, bool doRebuild_r )
{
  ///////////////////////////////////////////////////////////////////
  // Check arguments
  ///////////////////////////////////////////////////////////////////
  bool quickinit( root_r.empty() );

  if ( root_r.empty() )
    root_r = "/";

  const Pathname & dbPath_r { librpmDb::suggestedDbPath( root_r ) };	// also asserts root_r is absolute

  // The rpmdb compat symlink.
  // Required at least until rpmdb2solv takes a dppath argument.
  // Otherwise it creates a db at "/var/lib/rpm".
  if ( dbPath_r != "/var/lib/rpm" && ! PathInfo( root_r/"/var/lib/rpm" ).isExist() )
  {
    WAR << "Inject missing /var/lib/rpm compat symlink to " << dbPath_r << endl;
    filesystem::assert_dir( root_r/"/var/lib" );
    filesystem::symlink( "../../"/dbPath_r, root_r/"/var/lib/rpm" );
  }

  ///////////////////////////////////////////////////////////////////
  // Check whether already initialized
  ///////////////////////////////////////////////////////////////////
  if ( initialized() )
  {
    // Just check for a changing root because the librpmDb::suggestedDbPath
    // may indeed change: rpm %post moving the db from /var/lib/rpm
    // to /usr/lib/sysimage/rpm. We continue to use the old dbpath
    // (via the compat symlink) until a re-init.
    if ( root_r == _root ) {
      MIL << "Calling initDatabase: already initialized at " << stringPath( _root, _dbPath ) << endl;
      return;
    }
    else
      ZYPP_THROW(RpmDbAlreadyOpenException(_root, _dbPath, root_r, dbPath_r));
  }

  MIL << "Calling initDatabase: " << stringPath( root_r, dbPath_r )
      << ( doRebuild_r ? " (rebuilddb)" : "" )
      << ( quickinit ? " (quickinit)" : "" ) << endl;

  ///////////////////////////////////////////////////////////////////
  // init database
  ///////////////////////////////////////////////////////////////////
  librpmDb::unblockAccess();

  if ( quickinit )
  {
    MIL << "QUICK initDatabase (no systemRoot set)" << endl;
    return;
  }

  try
  {
    // creates dbdir and empty rpm database if not present
    librpmDb::dbAccess( root_r );
  }
  catch (const RpmException & excpt_r)
  {
    ZYPP_CAUGHT(excpt_r);
    librpmDb::blockAccess();
    ZYPP_RETHROW(excpt_r);
  }

  _root   = root_r;
  _dbPath = dbPath_r;

  if ( doRebuild_r )
    rebuildDatabase();

  MIL << "Synchronizing keys with zypp keyring" << endl;
  syncTrustedKeys();

  // Close the database in case any write acces (create/convert)
  // happened during init. This should drop any lock acquired
  // by librpm. On demand it will be reopened readonly and should
  // not hold any lock.
  librpmDb::dbRelease( true );

  MIL << "InitDatabase: " << *this << endl;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::closeDatabase
//	METHOD TYPE : PMError
//
void RpmDb::closeDatabase()
{
  if ( ! initialized() )
  {
    return;
  }

  MIL << "Calling closeDatabase: " << *this << endl;

  ///////////////////////////////////////////////////////////////////
  // Block further database access
  ///////////////////////////////////////////////////////////////////
  librpmDb::blockAccess();

  ///////////////////////////////////////////////////////////////////
  // Uninit
  ///////////////////////////////////////////////////////////////////
  _root = _dbPath = Pathname();

  MIL << "closeDatabase: " << *this << endl;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::rebuildDatabase
//	METHOD TYPE : PMError
//
void RpmDb::rebuildDatabase()
{
  callback::SendReport<RebuildDBReport> report;

  report->start( root() + dbPath() );

  try
  {
    doRebuildDatabase(report);
  }
  catch (RpmException & excpt_r)
  {
    report->finish(root() + dbPath(), RebuildDBReport::FAILED, excpt_r.asUserHistory());
    ZYPP_RETHROW(excpt_r);
  }
  report->finish(root() + dbPath(), RebuildDBReport::NO_ERROR, "");
}

void RpmDb::doRebuildDatabase(callback::SendReport<RebuildDBReport> & report)
{
  FAILIFNOTINITIALIZED;
  MIL << "RpmDb::rebuildDatabase" << *this << endl;

  const Pathname mydbpath { root()/dbPath() };	// the configured path used in reports
  {
    // For --rebuilddb take care we're using the real db directory
    // and not a symlink. Otherwise rpm will rename the symlink and
    // replace it with a real directory containing the converted db.
    DtorReset guardRoot  { _root };
    DtorReset guardDbPath{ _dbPath };
    _root = "/";
    _dbPath = filesystem::expandlink( mydbpath );

    // run rpm
    RpmArgVec opts;
    opts.push_back("--rebuilddb");
    opts.push_back("-vv");
    run_rpm (opts, ExternalProgram::Stderr_To_Stdout);
  }

  // generate and report progress
  ProgressData tics;
  {
    ProgressData::value_type hdrTotal = 0;
    for ( librpmDb::db_const_iterator it; *it; ++it, ++hdrTotal )
    {;}
    tics.range( hdrTotal );
  }
  tics.sendTo( [&report,&mydbpath]( const ProgressData & tics_r ) -> bool {
    return report->progress( tics_r.reportValue(), mydbpath );
  } );
  tics.toMin();

  std::string line;
  std::string errmsg;
  while ( systemReadLine( line ) )
  {
    static const std::string debugPrefix    { "D:" };
    static const std::string progressPrefix { "D:  read h#" };
    static const std::string ignoreSuffix   { "digest: OK" };

    if ( ! str::startsWith( line, debugPrefix ) )
    {
      if ( ! str::endsWith( line, ignoreSuffix ) )
      {
        errmsg += line;
        errmsg += '\n';
        WAR << line << endl;
      }
    }
    else if ( str::startsWith( line, progressPrefix ) )
    {
      if ( ! tics.incr() )
      {
        WAR << "User requested abort." << endl;
        systemKill();
      }
    }
  }

  if ( systemStatus() != 0 )
  {
    //TranslatorExplanation after semicolon is error message
    ZYPP_THROW(RpmSubprocessException(std::string(_("RPM failed: ")) + (errmsg.empty() ? error_message: errmsg) ) );
  }
  else
  {
    tics.toMax();
  }
}

///////////////////////////////////////////////////////////////////
namespace
{
  /** \ref RpmDb::syncTrustedKeys helper
   * Compute which keys need to be exprted to / imported from the zypp keyring.
   * Return result via argument list.
   */
  void computeKeyRingSync( std::set<Edition> & rpmKeys_r, std::list<PublicKeyData> & zyppKeys_r )
  {
    ///////////////////////////////////////////////////////////////////
    // Remember latest release and where it ocurred
    struct Key
    {
      Key()
        : _inRpmKeys( nullptr )
        , _inZyppKeys( nullptr )
      {}

      void updateIf( const Edition & rpmKey_r )
      {
        std::string keyRelease( rpmKey_r.release() );
        int comp = _release.compare( keyRelease );
        if ( comp < 0 )
        {
          // update to newer release
          _release.swap( keyRelease );
          _inRpmKeys  = &rpmKey_r;
          _inZyppKeys = nullptr;
          if ( !keyRelease.empty() )
            DBG << "Old key in Z: gpg-pubkey-" << rpmKey_r.version() << "-" <<  keyRelease << endl;
        }
        else if ( comp == 0 )
        {
          // stay with this release
          if ( ! _inRpmKeys )
            _inRpmKeys = &rpmKey_r;
        }
        // else: this is an old release
        else
          DBG << "Old key in R: gpg-pubkey-" << rpmKey_r.version() << "-" <<  keyRelease << endl;
      }

      void updateIf( const PublicKeyData & zyppKey_r )
      {
        std::string keyRelease( zyppKey_r.gpgPubkeyRelease() );
        int comp = _release.compare( keyRelease );
        if ( comp < 0 )
        {
          // update to newer release
          _release.swap( keyRelease );
          _inRpmKeys  = nullptr;
          _inZyppKeys = &zyppKey_r;
          if ( !keyRelease.empty() )
            DBG << "Old key in R: gpg-pubkey-" << zyppKey_r.gpgPubkeyVersion() << "-" << keyRelease << endl;
        }
        else if ( comp == 0 )
        {
          // stay with this release
          if ( ! _inZyppKeys )
            _inZyppKeys = &zyppKey_r;
        }
        // else: this is an old release
        else
          DBG << "Old key in Z: gpg-pubkey-" << zyppKey_r.gpgPubkeyVersion() << "-" << keyRelease << endl;
      }

      std::string _release;
      const Edition * _inRpmKeys;
      const PublicKeyData * _inZyppKeys;
    };
    ///////////////////////////////////////////////////////////////////

    // collect keys by ID(version) and latest creation(release)
    std::map<std::string,Key> _keymap;

    for_( it, rpmKeys_r.begin(), rpmKeys_r.end() )
    {
      _keymap[(*it).version()].updateIf( *it );
    }

    for_( it, zyppKeys_r.begin(), zyppKeys_r.end() )
    {
      _keymap[(*it).gpgPubkeyVersion()].updateIf( *it );
    }

    // compute missing keys
    std::set<Edition> rpmKeys;
    std::list<PublicKeyData> zyppKeys;
    for_( it, _keymap.begin(), _keymap.end() )
    {
      DBG << "gpg-pubkey-" << (*it).first << "-" << (*it).second._release << " "
          << ( (*it).second._inRpmKeys  ? "R" : "_" )
          << ( (*it).second._inZyppKeys ? "Z" : "_" ) << endl;
      if ( ! (*it).second._inRpmKeys )
      {
        zyppKeys.push_back( *(*it).second._inZyppKeys );
      }
      if ( ! (*it).second._inZyppKeys )
      {
        rpmKeys.insert( *(*it).second._inRpmKeys );
      }
    }
    rpmKeys_r.swap( rpmKeys );
    zyppKeys_r.swap( zyppKeys );
  }
} // namespace
///////////////////////////////////////////////////////////////////

void RpmDb::syncTrustedKeys( SyncTrustedKeyBits mode_r )
{
  MIL << "Going to sync trusted keys..." << endl;
  std::set<Edition> rpmKeys( pubkeyEditions() );
  std::list<PublicKeyData> zyppKeys( getZYpp()->keyRing()->trustedPublicKeyData() );

  if ( ! ( mode_r & SYNC_FROM_KEYRING ) )
  {
    // bsc#1064380: We relief PK from removing excess keys in the zypp keyring
    // when re-acquiring the zyppp lock. For now we remove all excess keys.
    // TODO: Once we can safely assume that all PK versions are updated we
    // can think about re-importing newer key versions found in the zypp keyring and
    // removing only excess ones (but case is not very likely). Unfixed PK versions
    // however will remove the newer version found in the zypp keyring and by doing
    // this, the key here will be removed via callback as well (keys are deleted
    // via gpg id, regardless of the edition).
    MIL << "Removing excess keys in zypp trusted keyring" << std::endl;
    // Temporarily disconnect to prevent the attempt to pass back the delete request.
    callback::TempConnect<KeyRingSignals> tempDisconnect;
    bool dirty = false;
    for ( const PublicKeyData & keyData : zyppKeys )
    {
      if ( ! rpmKeys.count( keyData.gpgPubkeyEdition() ) )
      {
        DBG << "Excess key in Z to delete: gpg-pubkey-" << keyData.gpgPubkeyEdition() << endl;
        getZYpp()->keyRing()->deleteKey( keyData.id(), /*trusted*/true );
        if ( !dirty ) dirty = true;
      }
    }
    if ( dirty )
      zyppKeys = getZYpp()->keyRing()->trustedPublicKeyData();
  }

  computeKeyRingSync( rpmKeys, zyppKeys );
  MIL << (mode_r & SYNC_TO_KEYRING   ? "" : "(skip) ") << "Rpm keys to export into zypp trusted keyring: " << rpmKeys.size() << endl;
  MIL << (mode_r & SYNC_FROM_KEYRING ? "" : "(skip) ") << "Zypp trusted keys to import into rpm database: " << zyppKeys.size() << endl;

  ///////////////////////////////////////////////////////////////////
  if ( (mode_r & SYNC_TO_KEYRING) &&  ! rpmKeys.empty() )
  {
    // export to zypp keyring
    MIL << "Exporting rpm keyring into zypp trusted keyring" <<endl;
    // Temporarily disconnect to prevent the attempt to re-import the exported keys.
    callback::TempConnect<KeyRingSignals> tempDisconnect;
    librpmDb::db_const_iterator keepDbOpen; // just to keep a ref.

    TmpFile tmpfile( getZYpp()->tmpPath() );
    {
      std::ofstream tmpos( tmpfile.path().c_str() );
      for_( it, rpmKeys.begin(), rpmKeys.end() )
      {
        // we export the rpm key into a file
        RpmHeader::constPtr result;
        getData( "gpg-pubkey", *it, result );
        tmpos << result->tag_description() << endl;
      }
    }
    try
    {
      getZYpp()->keyRing()->multiKeyImport( tmpfile.path(), true /*trusted*/);
      // bsc#1096217: Try to spot and report legacy V3 keys found in the rpm database.
      // Modern rpm does not import those keys, but when migrating a pre SLE12 system
      // we may find them. rpm>4.13 even complains on sderr if sucha key is present.
      std::set<Edition> missingKeys;
      for ( const Edition & key : rpmKeys )
      {
        if ( getZYpp()->keyRing()->isKeyTrusted( key.version() ) ) // key.version is the gpgkeys short ID
          continue;
        ERR << "Could not import key:" << str::Format("gpg-pubkey-%s") % key << " into zypp keyring (V3 key?)" << endl;
        missingKeys.insert( key );
      }
      if ( ! missingKeys.empty() )
        callback::SendReport<KeyRingReport>()->reportNonImportedKeys(missingKeys);
    }
    catch ( const Exception & excpt )
    {
      ZYPP_CAUGHT( excpt );
      ERR << "Could not import keys into zypp keyring: " << endl;
    }
  }

  ///////////////////////////////////////////////////////////////////
  if ( (mode_r & SYNC_FROM_KEYRING) && ! zyppKeys.empty() )
  {
    // import from zypp keyring
    MIL << "Importing zypp trusted keyring" << std::endl;
    for_( it, zyppKeys.begin(), zyppKeys.end() )
    {
      try
      {
        importPubkey( getZYpp()->keyRing()->exportTrustedPublicKey( *it ) );
      }
      catch ( const RpmException & exp )
      {
        ZYPP_CAUGHT( exp );
      }
    }
  }
  MIL << "Trusted keys synced." << endl;
}

void RpmDb::importZyppKeyRingTrustedKeys()
{ syncTrustedKeys( SYNC_FROM_KEYRING ); }

void RpmDb::exportTrustedKeysInZyppKeyRing()
{ syncTrustedKeys( SYNC_TO_KEYRING ); }

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::importPubkey
//	METHOD TYPE : PMError
//
void RpmDb::importPubkey( const PublicKey & pubkey_r )
{
  FAILIFNOTINITIALIZED;

  // bnc#828672: On the fly key import in READONLY
  if ( zypp_readonly_hack::IGotIt() )
  {
    WAR << "Key " << pubkey_r << " can not be imported. (READONLY MODE)" << endl;
    return;
  }

  // check if the key is already in the rpm database
  Edition keyEd( pubkey_r.gpgPubkeyVersion(), pubkey_r.gpgPubkeyRelease() );
  std::set<Edition> rpmKeys = pubkeyEditions();
  bool hasOldkeys = false;

  for_( it, rpmKeys.begin(), rpmKeys.end() )
  {
    // bsc#1008325: Keys using subkeys for signing don't get a higher release
    // if new subkeys are added, because the primary key remains unchanged.
    // For now always re-import keys with subkeys. Here we don't want to export the
    // keys in the rpm database to check whether the subkeys are the same. The calling
    // code should take care, we don't re-import the same kesy over and over again.
    if ( keyEd == *it && !pubkey_r.hasSubkeys() ) // quick test (Edition is IdStringType!)
    {
      MIL << "Key " << pubkey_r << " is already in the rpm trusted keyring. (skip import)" << endl;
      return;
    }

    if ( keyEd.version() != (*it).version() )
      continue; // different key ID (version)

    if ( keyEd.release() < (*it).release() )
    {
      MIL << "Key " << pubkey_r << " is older than one in the rpm trusted keyring. (skip import)" << endl;
      return;
    }
    else
    {
      hasOldkeys = true;
    }
  }
  MIL << "Key " << pubkey_r << " will be imported into the rpm trusted keyring." << (hasOldkeys?"(update)":"(new)") << endl;

  if ( hasOldkeys )
  {
    // We must explicitly delete old key IDs first (all releases,
    // that's why we don't call removePubkey here).
    std::string keyName( "gpg-pubkey-" + keyEd.version() );
    RpmArgVec opts;
    opts.push_back ( "-e" );
    opts.push_back ( "--allmatches" );
    opts.push_back ( "--" );
    opts.push_back ( keyName.c_str() );
    run_rpm( opts, ExternalProgram::Stderr_To_Stdout );

    std::string line;
    while ( systemReadLine( line ) )
    {
      ( str::startsWith( line, "error:" ) ? WAR : DBG ) << line << endl;
    }

    if ( systemStatus() != 0 )
    {
      ERR << "Failed to remove key " << pubkey_r << " from RPM trusted keyring (ignored)" << endl;
    }
    else
    {
      MIL << "Key " << pubkey_r << " has been removed from RPM trusted keyring" << endl;
    }
  }

  // import the new key
  RpmArgVec opts;
  opts.push_back ( "--import" );
  opts.push_back ( "--" );
  std::string pubkeypath( pubkey_r.path().asString() );
  opts.push_back ( pubkeypath.c_str() );
  run_rpm( opts, ExternalProgram::Stderr_To_Stdout );

  std::string line;
  std::vector<std::string> excplines;
  while ( systemReadLine( line ) )
  {
    if ( str::startsWith( line, "error:" ) )
    {
      WAR << line << endl;
      excplines.push_back( std::move(line) );
    }
    else
      DBG << line << endl;
  }

  if ( systemStatus() != 0 )
  {
    // Translator: %1% is a gpg public key
    RpmSubprocessException excp( str::Format(_("Failed to import public key %1%") ) % pubkey_r.asString() );
    excp.moveToHistory( excplines );
    excp.addHistory( std::move(error_message) );
    ZYPP_THROW( std::move(excp) );
  }
  else
  {
    MIL << "Key " << pubkey_r << " imported in rpm trusted keyring." << endl;
  }
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::removePubkey
//	METHOD TYPE : PMError
//
void RpmDb::removePubkey( const PublicKey & pubkey_r )
{
  FAILIFNOTINITIALIZED;

  // check if the key is in the rpm database and just
  // return if it does not.
  std::set<Edition> rpm_keys = pubkeyEditions();
  std::set<Edition>::const_iterator found_edition = rpm_keys.end();
  std::string pubkeyVersion( pubkey_r.gpgPubkeyVersion() );

  for_( it, rpm_keys.begin(), rpm_keys.end() )
  {
    if ( (*it).version() == pubkeyVersion )
    {
        found_edition = it;
        break;
    }
  }

  // the key does not exist, cannot be removed
  if (found_edition == rpm_keys.end())
  {
      WAR << "Key " << pubkey_r.id() << " is not in rpm db" << endl;
      return;
  }

  std::string rpm_name("gpg-pubkey-" + found_edition->asString());

  RpmArgVec opts;
  opts.push_back ( "-e" );
  opts.push_back ( "--" );
  opts.push_back ( rpm_name.c_str() );
  run_rpm( opts, ExternalProgram::Stderr_To_Stdout );

  std::string line;
  std::vector<std::string> excplines;
  while ( systemReadLine( line ) )
  {
    if ( str::startsWith( line, "error:" ) )
    {
      WAR << line << endl;
      excplines.push_back( std::move(line) );
    }
    else
      DBG << line << endl;
  }

  if ( systemStatus() != 0 )
  {
    // Translator: %1% is a gpg public key
    RpmSubprocessException excp( str::Format(_("Failed to remove public key %1%") ) % pubkey_r.asString() );
    excp.moveToHistory( excplines );
    excp.addHistory( std::move(error_message) );
    ZYPP_THROW( std::move(excp) );
  }
  else
  {
    MIL << "Key " << pubkey_r << " has been removed from RPM trusted keyring" << endl;
  }
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::pubkeys
//	METHOD TYPE : std::set<Edition>
//
std::list<PublicKey> RpmDb::pubkeys() const
{
  std::list<PublicKey> ret;

  librpmDb::db_const_iterator it;
  for ( it.findByName( "gpg-pubkey" ); *it; ++it )
  {
    Edition edition = it->tag_edition();
    if (edition != Edition::noedition)
    {
      // we export the rpm key into a file
      RpmHeader::constPtr result;
      getData( "gpg-pubkey", edition, result );
      TmpFile file(getZYpp()->tmpPath());
      std::ofstream os;
      try
      {
        os.open(file.path().asString().c_str());
        // dump rpm key into the tmp file
        os << result->tag_description();
        //MIL << "-----------------------------------------------" << endl;
        //MIL << result->tag_description() <<endl;
        //MIL << "-----------------------------------------------" << endl;
        os.close();
        // read the public key from the dumped file
        PublicKey key(file);
        ret.push_back(key);
      }
      catch ( std::exception & e )
      {
        ERR << "Could not dump key " << edition.asString() << " in tmp file " << file.path() << endl;
        // just ignore the key
      }
    }
  }
  return ret;
}

std::set<Edition> RpmDb::pubkeyEditions() const
  {
    std::set<Edition> ret;

    librpmDb::db_const_iterator it;
    for ( it.findByName( "gpg-pubkey" ); *it; ++it )
    {
      Edition edition = it->tag_edition();
      if (edition != Edition::noedition)
        ret.insert( edition );
    }
    return ret;
  }


///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::fileList
//	METHOD TYPE : bool
//
//	DESCRIPTION :
//
std::list<FileInfo>
RpmDb::fileList( const std::string & name_r, const Edition & edition_r ) const
{
  std::list<FileInfo> result;

  librpmDb::db_const_iterator it;
  bool found;
  if (edition_r == Edition::noedition)
  {
    found = it.findPackage( name_r );
  }
  else
  {
    found = it.findPackage( name_r, edition_r );
  }
  if (!found)
    return result;

  return result;
}


///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::hasFile
//	METHOD TYPE : bool
//
//	DESCRIPTION :
//
bool RpmDb::hasFile( const std::string & file_r, const std::string & name_r ) const
{
  librpmDb::db_const_iterator it;
  bool res;
  do
  {
    res = it.findByFile( file_r );
    if (!res) break;
    if (!name_r.empty())
    {
      res = (it->tag_name() == name_r);
    }
    ++it;
  }
  while (res && *it);
  return res;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::whoOwnsFile
//	METHOD TYPE : std::string
//
//	DESCRIPTION :
//
std::string RpmDb::whoOwnsFile( const std::string & file_r) const
{
  librpmDb::db_const_iterator it;
  if (it.findByFile( file_r ))
  {
    return it->tag_name();
  }
  return "";
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::hasProvides
//	METHOD TYPE : bool
//
//	DESCRIPTION :
//
bool RpmDb::hasProvides( const std::string & tag_r ) const
{
  librpmDb::db_const_iterator it;
  return it.findByProvides( tag_r );
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::hasRequiredBy
//	METHOD TYPE : bool
//
//	DESCRIPTION :
//
bool RpmDb::hasRequiredBy( const std::string & tag_r ) const
{
  librpmDb::db_const_iterator it;
  return it.findByRequiredBy( tag_r );
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::hasConflicts
//	METHOD TYPE : bool
//
//	DESCRIPTION :
//
bool RpmDb::hasConflicts( const std::string & tag_r ) const
{
  librpmDb::db_const_iterator it;
  return it.findByConflicts( tag_r );
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::hasPackage
//	METHOD TYPE : bool
//
//	DESCRIPTION :
//
bool RpmDb::hasPackage( const std::string & name_r ) const
{
  librpmDb::db_const_iterator it;
  return it.findPackage( name_r );
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::hasPackage
//	METHOD TYPE : bool
//
//	DESCRIPTION :
//
bool RpmDb::hasPackage( const std::string & name_r, const Edition & ed_r ) const
{
  librpmDb::db_const_iterator it;
  return it.findPackage( name_r, ed_r );
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::getData
//	METHOD TYPE : PMError
//
//	DESCRIPTION :
//
void RpmDb::getData( const std::string & name_r,
                     RpmHeader::constPtr & result_r ) const
{
  librpmDb::db_const_iterator it;
  it.findPackage( name_r );
  result_r = *it;
  if (it.dbError())
    ZYPP_THROW(*(it.dbError()));
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::getData
//	METHOD TYPE : void
//
//	DESCRIPTION :
//
void RpmDb::getData( const std::string & name_r, const Edition & ed_r,
                     RpmHeader::constPtr & result_r ) const
{
  librpmDb::db_const_iterator it;
  it.findPackage( name_r, ed_r  );
  result_r = *it;
  if (it.dbError())
    ZYPP_THROW(*(it.dbError()));
}

///////////////////////////////////////////////////////////////////
namespace
{
  struct RpmlogCapture : public std::string
  {
    RpmlogCapture() : _cap(this)
    {
      rpmlogSetCallback( rpmLogCB, this );
      _oldMask = rpmlogSetMask( RPMLOG_UPTO( RPMLOG_PRI(RPMLOG_INFO) ) );
    }

    ~RpmlogCapture()
    {
      rpmlogSetCallback( nullptr, nullptr );
      rpmlogSetMask( _oldMask );
    }

    static int rpmLogCB( rpmlogRec rec_r, rpmlogCallbackData data_r )
    { return reinterpret_cast<RpmlogCapture*>(data_r)->rpmLog( rec_r ); }

    int rpmLog( rpmlogRec rec_r )
    {
      if ( _cap ) (*_cap) += rpmlogRecMessage( rec_r );
      return 0;
    }

  private:
    std::string * _cap;
    int _oldMask = 0;
  };

  RpmDb::CheckPackageResult doCheckPackageSig( const Pathname & path_r,			// rpm file to check
                                               const Pathname & root_r,			// target root
                                               bool  requireGPGSig_r,			// whether no gpg signature is to be reported
                                               RpmDb::CheckPackageDetail & detail_r )	// detailed result
  {
    PathInfo file( path_r );
    if ( ! file.isFile() )
    {
      ERR << "Not a file: " << file << endl;
      return RpmDb::CHK_ERROR;
    }

    FD_t fd = ::Fopen( file.asString().c_str(), "r.ufdio" );
    if ( fd == 0 || ::Ferror(fd) )
    {
      ERR << "Can't open file for reading: " << file << " (" << ::Fstrerror(fd) << ")" << endl;
      if ( fd )
        ::Fclose( fd );
      return RpmDb::CHK_ERROR;
    }
    rpmts ts = ::rpmtsCreate();
    ::rpmtsSetRootDir( ts, root_r.c_str() );
    ::rpmtsSetVSFlags( ts, RPMVSF_DEFAULT );

    rpmQVKArguments_s qva;
    memset( &qva, 0, sizeof(rpmQVKArguments_s) );
#ifndef HAVE_RPM_VERIFY_TRANSACTION_STEP
    // Legacy: In rpm >= 4.15 qva_flags symbols don't exist
    // and qva_flags is not used in signature checking at all.
    qva.qva_flags = (VERIFY_DIGEST|VERIFY_SIGNATURE);
#else
    ::rpmtsSetVfyFlags( ts, RPMVSF_DEFAULT );
#endif
    RpmlogCapture vresult;
    LocaleGuard guard( LC_ALL, "C" );	// bsc#1076415: rpm log output is localized, but we need to parse it :(
    int res = ::rpmVerifySignatures( &qva, ts, fd, path_r.basename().c_str() );
    guard.restore();

    ts = rpmtsFree(ts);
    ::Fclose( fd );

    // results per line...
    //     Header V3 RSA/SHA256 Signature, key ID 3dbdc284: OK
    //     Header SHA1 digest: OK (a60386347863affefef484ff1f26c889373eb094)
    //     V3 RSA/SHA256 Signature, key ID 3dbdc284: OK
    //     MD5 digest: OK (fd5259fe677a406951dcb2e9d08c4dcc)
    //
    std::vector<std::string> lines;
    str::split( vresult, std::back_inserter(lines), "\n" );
    unsigned count[7] = { 0, 0, 0, 0, 0, 0, 0 };

    for ( unsigned i = 1; i < lines.size(); ++i )
    {
      std::string &line( lines[i] );

      RpmDb::CheckPackageResult lineres = RpmDb::CHK_ERROR;
      if ( line.find( ": OK" ) != std::string::npos )
      {
        lineres = RpmDb::CHK_OK;
        if ( line.find( "Signature, key ID" ) == std::string::npos )
          ++count[RpmDb::CHK_NOSIG];	// Valid but no gpg signature -> CHK_NOSIG
      }
      else if ( line.find( ": NOKEY" ) != std::string::npos )
      { lineres = RpmDb::CHK_NOKEY; }
      else if ( line.find( ": BAD" ) != std::string::npos )
      { lineres = RpmDb::CHK_FAIL; }
      else if ( line.find( ": UNKNOWN" ) != std::string::npos )
      { lineres = RpmDb::CHK_NOTFOUND; }
      else if ( line.find( ": NOTRUSTED" ) != std::string::npos )
      { lineres = RpmDb::CHK_NOTTRUSTED; }
      else if ( line.find( ": NOTFOUND" ) != std::string::npos )
      { continue; } // just collect details for signatures found (#229)

      ++count[lineres];
      detail_r.push_back( RpmDb::CheckPackageDetail::value_type( lineres, std::move(line) ) );
    }

    RpmDb::CheckPackageResult ret = ( res ? RpmDb::CHK_ERROR : RpmDb::CHK_OK );

    if ( count[RpmDb::CHK_FAIL] )
      ret = RpmDb::CHK_FAIL;

    else if ( count[RpmDb::CHK_NOTFOUND] )
      ret = RpmDb::CHK_NOTFOUND;

    else if ( count[RpmDb::CHK_NOKEY] )
      ret = RpmDb::CHK_NOKEY;

    else if ( count[RpmDb::CHK_NOTTRUSTED] )
      ret = RpmDb::CHK_NOTTRUSTED;

    else if ( ret == RpmDb::CHK_OK )
    {
      if ( count[RpmDb::CHK_OK] == count[RpmDb::CHK_NOSIG]  )
      {
        detail_r.push_back( RpmDb::CheckPackageDetail::value_type( RpmDb::CHK_NOSIG, std::string("    ")+_("Package is not signed!") ) );
        if ( requireGPGSig_r )
          ret = RpmDb::CHK_NOSIG;
      }
    }

    if ( ret != RpmDb::CHK_OK )
    {

      bool didReadHeader = false;
      std::unordered_map< std::string, std::string> fprs;

      // we replace the data only if the key IDs are actually only 8 bytes
      str::regex rxexpr( "key ID ([a-fA-F0-9]{8}):" );
      for ( auto &detail : detail_r ) {
        auto &line = detail.second;
        str::smatch what;
        if ( str::regex_match( line, what, rxexpr ) ) {

          if ( !didReadHeader ) {
            didReadHeader = true;

            // Get signature info from the package header, RPM always prints only the 8 byte ID
            auto header = RpmHeader::readPackage( path_r, RpmHeader::NOVERIFY );
            if ( header ) {
              auto keyMgr = zypp::KeyManagerCtx::createForOpenPGP();
              const auto &addFprs = [&]( auto tag ){
                const auto &list1 = keyMgr.readSignatureFingerprints( header->blob_val( tag ) );
                for ( const auto &id : list1 ) {
                  if ( id.size() <= 8 )
                    continue;

                  const auto &lowerId = str::toLower( id );
                  fprs.insert( std::make_pair( lowerId.substr( lowerId.size() - 8 ), lowerId ) );
                }
              };

              addFprs( RPMTAG_SIGGPG );
              addFprs( RPMTAG_SIGPGP );
              addFprs( RPMTAG_RSAHEADER );
              addFprs( RPMTAG_DSAHEADER );

            } else {
              ERR << "Failed to read package signatures." << std::endl;
            }
          }

          // if we have no keys we can substiture we can leave the loop right away
          if ( !fprs.size() )
            break;

          {
            // replace the short key ID with the long ones parsed from the header
            const auto &keyId = str::toLower( what[1] );
            if ( const auto &i = fprs.find( keyId ); i != fprs.end() ) {
              str::replaceAll( line, keyId, i->second );
            }
          }
        }
      }

      WAR << path_r << " (" << requireGPGSig_r << " -> " << ret << ")" << endl;
      WAR << vresult;
    }
    else
      DBG << path_r << " [0-Signature is OK]" << endl;
    return ret;
  }

} // namespace
///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : RpmDb::checkPackage
//	METHOD TYPE : RpmDb::CheckPackageResult
//
RpmDb::CheckPackageResult RpmDb::checkPackage( const Pathname & path_r, CheckPackageDetail & detail_r )
{ return doCheckPackageSig( path_r, root(), false/*requireGPGSig_r*/, detail_r ); }

RpmDb::CheckPackageResult RpmDb::checkPackage( const Pathname & path_r )
{ CheckPackageDetail dummy; return checkPackage( path_r, dummy ); }

RpmDb::CheckPackageResult RpmDb::checkPackageSignature( const Pathname & path_r, RpmDb::CheckPackageDetail & detail_r )
{ return doCheckPackageSig( path_r, root(), true/*requireGPGSig_r*/, detail_r ); }


// determine changed files of installed package
bool
RpmDb::queryChangedFiles(FileList & fileList, const std::string& packageName)
{
  bool ok = true;

  fileList.clear();

  if ( ! initialized() ) return false;

  RpmArgVec opts;

  opts.push_back ("-V");
  opts.push_back ("--nodeps");
  opts.push_back ("--noscripts");
  opts.push_back ("--nomd5");
  opts.push_back ("--");
  opts.push_back (packageName.c_str());

  run_rpm (opts, ExternalProgram::Discard_Stderr);

  if ( process == NULL )
    return false;

  /* from rpm manpage
   5      MD5 sum
   S      File size
   L      Symlink
   T      Mtime
   D      Device
   U      User
   G      Group
   M      Mode (includes permissions and file type)
  */

  std::string line;
  while (systemReadLine(line))
  {
    if (line.length() > 12 &&
        (line[0] == 'S' || line[0] == 's' ||
         (line[0] == '.' && line[7] == 'T')))
    {
      // file has been changed
      std::string filename;

      filename.assign(line, 11, line.length() - 11);
      fileList.insert(filename);
    }
  }

  systemStatus();
  // exit code ignored, rpm returns 1 no matter if package is installed or
  // not

  return ok;
}


/****************************************************************/
/* private member-functions					*/
/****************************************************************/

/*--------------------------------------------------------------*/
/* Run rpm with the specified arguments, handling stderr	*/
/* as specified  by disp					*/
/*--------------------------------------------------------------*/
void
RpmDb::run_rpm (const RpmArgVec& opts,
                ExternalProgram::Stderr_Disposition disp)
{
  if ( process )
  {
    delete process;
    process = NULL;
  }
  exit_code = -1;

  if ( ! initialized() )
  {
    ZYPP_THROW(RpmDbNotOpenException());
  }

  RpmArgVec args;

  // always set root and dbpath
#if defined(WORKAROUNDRPMPWDBUG)
  args.push_back("#/");		// chdir to / to workaround bnc#819354
#endif
  args.push_back("rpm");
  args.push_back("--root");
  args.push_back(_root.asString().c_str());
  args.push_back("--dbpath");
  args.push_back(_dbPath.asString().c_str());
  if ( env::ZYPP_RPM_DEBUG() )
    args.push_back("-vv");
  const char* argv[args.size() + opts.size() + 1];

  const char** p = argv;
  p = copy (args.begin (), args.end (), p);
  p = copy (opts.begin (), opts.end (), p);
  *p = 0;

  // Invalidate all outstanding database handles in case
  // the database gets modified.
  librpmDb::dbRelease( true );

  // Launch the program with default locale
  process = new ExternalProgram(argv, disp, false, -1, true);
  return;
}

/*--------------------------------------------------------------*/
/* Read a line from the rpm process				*/
/*--------------------------------------------------------------*/
bool RpmDb::systemReadLine( std::string & line )
{
  line.erase();

  if ( process == NULL )
    return false;

  if ( process->inputFile() )
  {
    process->setBlocking( false );
    FILE * inputfile = process->inputFile();
    do {
      // Check every 5 seconds if the process is still running to prevent against
      // daemons launched in rpm %post that do not close their filedescriptors,
      // causing us to block for infinity. (bnc#174548)
      const auto &readResult = io::receiveUpto( inputfile, '\n', 5 * 1000, false );
      switch ( readResult.first ) {
        case io::ReceiveUpToResult::Timeout: {
          if ( !process->running() )
            return false;

          // we might have received a partial line, lets not forget about it
          line += readResult.second;
        }
        case io::ReceiveUpToResult::Error:
        case io::ReceiveUpToResult::EndOfFile: {
          line += readResult.second;
          if ( line.size() && line.back() == '\n')
            line.pop_back();
          return line.size(); // in case of pending output
        }
        case io::ReceiveUpToResult::Success: {
          line += readResult.second;

          if ( line.size() && line.back() == '\n')
            line.pop_back();

          if ( env::ZYPP_RPM_DEBUG() )
            L_DBG("RPM_DEBUG") << line << endl;
          return true; // complete line
        }
      }
    } while( true );
  }
  return false;
}

/*--------------------------------------------------------------*/
/* Return the exit status of the rpm process, closing the	*/
/* connection if not already done				*/
/*--------------------------------------------------------------*/
int
RpmDb::systemStatus()
{
  if ( process == NULL )
    return -1;

  exit_code = process->close();
  if (exit_code == 0)
    error_message = "";
  else
    error_message = process->execError();
  process->kill();
  delete process;
  process = 0;

  //   DBG << "exit code " << exit_code << endl;

  return exit_code;
}

/*--------------------------------------------------------------*/
/* Forcably kill the rpm process				*/
/*--------------------------------------------------------------*/
void
RpmDb::systemKill()
{
  if (process) process->kill();
}


// generate diff mails for config files
void RpmDb::processConfigFiles(const std::string& line, const std::string& name, const char* typemsg, const char* difffailmsg, const char* diffgenmsg)
{
  std::string msg = line.substr(9);
  std::string::size_type pos1 = std::string::npos;
  std::string::size_type pos2 = std::string::npos;
  std::string file1s, file2s;
  Pathname file1;
  Pathname file2;

  pos1 = msg.find (typemsg);
  for (;;)
  {
    if ( pos1 == std::string::npos )
      break;

    pos2 = pos1 + strlen (typemsg);

    if (pos2 >= msg.length() )
      break;

    file1 = msg.substr (0, pos1);
    file2 = msg.substr (pos2);

    file1s = file1.asString();
    file2s = file2.asString();

    if (!_root.empty() && _root != "/")
    {
      file1 = _root + file1;
      file2 = _root + file2;
    }

    std::string out;
    int ret = diffFiles (file1.asString(), file2.asString(), out, 25);
    if (ret)
    {
      Pathname file = _root + WARNINGMAILPATH;
      if (filesystem::assert_dir(file) != 0)
      {
        ERR << "Could not create " << file.asString() << endl;
        break;
      }
      file += Date(Date::now()).form("config_diff_%Y_%m_%d.log");
      std::ofstream notify(file.asString().c_str(), std::ios::out|std::ios::app);
      if (!notify)
      {
        ERR << "Could not open " <<  file << endl;
        break;
      }

      // Translator: %s = name of an rpm package. A list of diffs follows
      // this message.
      notify << str::form(_("Changed configuration files for %s:"), name.c_str()) << endl;
      if (ret>1)
      {
        ERR << "diff failed" << endl;
        notify << str::form(difffailmsg,
                            file1s.c_str(), file2s.c_str()) << endl;
      }
      else
      {
        notify << str::form(diffgenmsg,
                            file1s.c_str(), file2s.c_str()) << endl;

        // remove root for the viewer's pleasure (#38240)
        if (!_root.empty() && _root != "/")
        {
          if (out.substr(0,4) == "--- ")
          {
            out.replace(4, file1.asString().length(), file1s);
          }
          std::string::size_type pos = out.find("\n+++ ");
          if (pos != std::string::npos)
          {
            out.replace(pos+5, file2.asString().length(), file2s);
          }
        }
        notify << out << endl;
      }
      notify.close();
      notify.open("/var/lib/update-messages/yast2-packagemanager.rpmdb.configfiles");
      notify.close();
    }
    else
    {
      WAR << "rpm created " << file2 << " but it is not different from " << file2 << endl;
    }
    break;
  }
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::installPackage
//	METHOD TYPE : PMError
//
void RpmDb::installPackage( const Pathname & filename, RpmInstFlags flags )
{
  callback::SendReport<RpmInstallReport> report;

  report->start(filename);

  do
    try
    {
      doInstallPackage(filename, flags, report);
      report->finish();
      break;
    }
    catch (RpmException & excpt_r)
    {
      RpmInstallReport::Action user = report->problem( excpt_r );

      if ( user == RpmInstallReport::ABORT )
      {
        report->finish( excpt_r );
        ZYPP_RETHROW(excpt_r);
      }
      else if ( user == RpmInstallReport::IGNORE )
      {
        break;
      }
    }
  while (true);
}

void RpmDb::doInstallPackage( const Pathname & filename, RpmInstFlags flags, callback::SendReport<RpmInstallReport> & report )
{
  FAILIFNOTINITIALIZED;
  HistoryLog historylog;

  MIL << "RpmDb::installPackage(" << filename << "," << flags << ")" << endl;

  // backup
  if ( _packagebackups )
  {
    // FIXME      report->progress( pd.init( -2, 100 ) ); // allow 1% for backup creation.
    if ( ! backupPackage( filename ) )
    {
      ERR << "backup of " << filename.asString() << " failed" << endl;
    }
    // FIXME status handling
    report->progress( 0 ); // allow 1% for backup creation.
  }

  // run rpm
  RpmArgVec opts;
  if (flags & RPMINST_NOUPGRADE)
    opts.push_back("-i");
  else
    opts.push_back("-U");

  opts.push_back("--percent");
  opts.push_back("--noglob");

  // ZConfig defines cross-arch installation
  if ( ! ZConfig::instance().systemArchitecture().compatibleWith( ZConfig::instance().defaultSystemArchitecture() ) )
    opts.push_back("--ignorearch");

  if (flags & RPMINST_NODIGEST)
    opts.push_back("--nodigest");
  if (flags & RPMINST_NOSIGNATURE)
    opts.push_back("--nosignature");
  if (flags & RPMINST_EXCLUDEDOCS)
    opts.push_back ("--excludedocs");
  if (flags & RPMINST_NOSCRIPTS)
    opts.push_back ("--noscripts");
  if (flags & RPMINST_FORCE)
    opts.push_back ("--force");
  if (flags & RPMINST_NODEPS)
    opts.push_back ("--nodeps");
  if (flags & RPMINST_IGNORESIZE)
    opts.push_back ("--ignoresize");
  if (flags & RPMINST_JUSTDB)
    opts.push_back ("--justdb");
  if (flags & RPMINST_TEST)
    opts.push_back ("--test");
  if (flags & RPMINST_NOPOSTTRANS)
    opts.push_back ("--noposttrans");

  opts.push_back("--");

  // rpm requires additional quoting of special chars:
  std::string quotedFilename( rpmQuoteFilename( workaroundRpmPwdBug( filename ) ) );
  opts.push_back ( quotedFilename.c_str() );
  run_rpm( opts, ExternalProgram::Stderr_To_Stdout );

  // forward additional rpm output via report;
  std::string line;
  unsigned    lineno = 0;
  callback::UserData cmdout( InstallResolvableReport::contentRpmout );
  // Key "solvable" injected by RpmInstallPackageReceiver
  cmdout.set( "line",   std::cref(line) );
  cmdout.set( "lineno", lineno );

  // LEGACY: collect and forward additional rpm output in finish
  std::string rpmmsg;
  std::vector<std::string> configwarnings;	// TODO: immediately process lines rather than collecting

  while ( systemReadLine( line ) )
  {
    if ( str::startsWith( line, "%%" ) )
    {
      int percent;
      sscanf( line.c_str() + 2, "%d", &percent );
      report->progress( percent );
      continue;
    }
    ++lineno;
    cmdout.set( "lineno", lineno );
    report->report( cmdout );

    if ( lineno >= MAXRPMMESSAGELINES ) {
      if ( line.find( " scriptlet failed, " ) == std::string::npos )	// always log %script errors
        continue;
    }

    rpmmsg += line+'\n';

    if ( str::startsWith( line, "warning:" ) )
      configwarnings.push_back(line);
  }
  if ( lineno >= MAXRPMMESSAGELINES )
    rpmmsg += "[truncated]\n";

  int rpm_status = systemStatus();

  // evaluate result
  for (std::vector<std::string>::iterator it = configwarnings.begin();
       it != configwarnings.end(); ++it)
  {
    processConfigFiles(*it, Pathname::basename(filename), " saved as ",
                       // %s = filenames
                       _("rpm saved %s as %s, but it was impossible to determine the difference"),
                       // %s = filenames
                       _("rpm saved %s as %s.\nHere are the first 25 lines of difference:\n"));
    processConfigFiles(*it, Pathname::basename(filename), " created as ",
                       // %s = filenames
                       _("rpm created %s as %s, but it was impossible to determine the difference"),
                       // %s = filenames
                       _("rpm created %s as %s.\nHere are the first 25 lines of difference:\n"));
  }

  if ( rpm_status != 0 )
  {
    historylog.comment(
        str::form("%s install failed", Pathname::basename(filename).c_str()),
        true /*timestamp*/);
    std::ostringstream sstr;
    sstr << "rpm output:" << endl << rpmmsg << endl;
    historylog.comment(sstr.str());
    // TranslatorExplanation the colon is followed by an error message
    auto excpt { RpmSubprocessException(_("RPM failed: ") + error_message ) };
    if ( not rpmmsg.empty() )
      excpt.addHistory( rpmmsg );
    ZYPP_THROW(std::move(excpt));
  }
  else if ( ! rpmmsg.empty() )
  {
    historylog.comment(
        str::form("%s installed ok", Pathname::basename(filename).c_str()),
        true /*timestamp*/);
    std::ostringstream sstr;
    sstr << "Additional rpm output:" << endl << rpmmsg << endl;
    historylog.comment(sstr.str());

    // report additional rpm output in finish (LEGACY! Lines are immediately reported as InstallResolvableReport::contentRpmout)
    // TranslatorExplanation Text is followed by a ':'  and the actual output.
    report->finishInfo(str::form( "%s:\n%s\n", _("Additional rpm output"),  rpmmsg.c_str() ));
  }
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::removePackage
//	METHOD TYPE : PMError
//
void RpmDb::removePackage( Package::constPtr package, RpmInstFlags flags )
{
  // 'rpm -e' does not like epochs
  return removePackage( package->name()
                        + "-" + package->edition().version()
                        + "-" + package->edition().release()
                        + "." + package->arch().asString(), flags );
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::removePackage
//	METHOD TYPE : PMError
//
void RpmDb::removePackage( const std::string & name_r, RpmInstFlags flags )
{
  callback::SendReport<RpmRemoveReport> report;

  report->start( name_r );

  do
    try
    {
      doRemovePackage(name_r, flags, report);
      report->finish();
      break;
    }
    catch (RpmException & excpt_r)
    {
      RpmRemoveReport::Action user = report->problem( excpt_r );

      if ( user == RpmRemoveReport::ABORT )
      {
        report->finish( excpt_r );
        ZYPP_RETHROW(excpt_r);
      }
      else if ( user == RpmRemoveReport::IGNORE )
      {
        break;
      }
    }
  while (true);
}


void RpmDb::doRemovePackage( const std::string & name_r, RpmInstFlags flags, callback::SendReport<RpmRemoveReport> & report )
{
  FAILIFNOTINITIALIZED;
  HistoryLog historylog;

  MIL << "RpmDb::doRemovePackage(" << name_r << "," << flags << ")" << endl;

  // backup
  if ( _packagebackups )
  {
    // FIXME solve this status report somehow
    //      report->progress( pd.init( -2, 100 ) ); // allow 1% for backup creation.
    if ( ! backupPackage( name_r ) )
    {
      ERR << "backup of " << name_r << " failed" << endl;
    }
    report->progress( 0 );
  }
  else
  {
    report->progress( 100 );
  }

  // run rpm
  RpmArgVec opts;
  opts.push_back("-e");
  opts.push_back("--allmatches");

  if (flags & RPMINST_NOSCRIPTS)
    opts.push_back("--noscripts");
  if (flags & RPMINST_NODEPS)
    opts.push_back("--nodeps");
  if (flags & RPMINST_JUSTDB)
    opts.push_back("--justdb");
  if (flags & RPMINST_TEST)
    opts.push_back ("--test");
  if (flags & RPMINST_FORCE)
  {
    WAR << "IGNORE OPTION: 'rpm -e' does not support '--force'" << endl;
  }

  opts.push_back("--");
  opts.push_back(name_r.c_str());
  run_rpm (opts, ExternalProgram::Stderr_To_Stdout);

  // forward additional rpm output via report;
  std::string line;
  unsigned    lineno = 0;
  callback::UserData cmdout( RemoveResolvableReport::contentRpmout );
  // Key "solvable" injected by RpmInstallPackageReceiver
  cmdout.set( "line",   std::cref(line) );
  cmdout.set( "lineno", lineno );


  // LEGACY: collect and forward additional rpm output in finish
  std::string rpmmsg;

  // got no progress from command, so we fake it:
  // 5  - command started
  // 50 - command completed
  // 100 if no error
  report->progress( 5 );
  while (systemReadLine(line))
  {
    ++lineno;
    cmdout.set( "lineno", lineno );
    report->report( cmdout );

    if ( lineno >= MAXRPMMESSAGELINES ) {
      if ( line.find( " scriptlet failed, " ) == std::string::npos )	// always log %script errors
        continue;
    }
    rpmmsg += line+'\n';
  }
  if ( lineno >= MAXRPMMESSAGELINES )
    rpmmsg += "[truncated]\n";
  report->progress( 50 );
  int rpm_status = systemStatus();

  if ( rpm_status != 0 )
  {
    historylog.comment(
        str::form("%s remove failed", name_r.c_str()), true /*timestamp*/);
    std::ostringstream sstr;
    sstr << "rpm output:" << endl << rpmmsg << endl;
    historylog.comment(sstr.str());
    // TranslatorExplanation the colon is followed by an error message
    auto excpt { RpmSubprocessException(_("RPM failed: ") + error_message ) };
    if ( not rpmmsg.empty() )
      excpt.addHistory( rpmmsg );
    ZYPP_THROW(std::move(excpt));
  }
  else if ( ! rpmmsg.empty() )
  {
    historylog.comment(
        str::form("%s removed ok", name_r.c_str()), true /*timestamp*/);

    std::ostringstream sstr;
    sstr << "Additional rpm output:" << endl << rpmmsg << endl;
    historylog.comment(sstr.str());

    // report additional rpm output in finish (LEGACY! Lines are immediately reported as RemoveResolvableReport::contentRpmout)
    // TranslatorExplanation Text is followed by a ':'  and the actual output.
    report->finishInfo(str::form( "%s:\n%s\n", _("Additional rpm output"),  rpmmsg.c_str() ));
  }
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::backupPackage
//	METHOD TYPE : bool
//
bool RpmDb::backupPackage( const Pathname & filename )
{
  RpmHeader::constPtr h( RpmHeader::readPackage( filename, RpmHeader::NOSIGNATURE ) );
  if ( ! h )
    return false;

  return backupPackage( h->tag_name() );
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::backupPackage
//	METHOD TYPE : bool
//
bool RpmDb::backupPackage(const std::string& packageName)
{
  HistoryLog progresslog;
  bool ret = true;
  Pathname backupFilename;
  Pathname filestobackupfile = _root+_backuppath+FILEFORBACKUPFILES;

  if (_backuppath.empty())
  {
    INT << "_backuppath empty" << endl;
    return false;
  }

  FileList fileList;

  if (!queryChangedFiles(fileList, packageName))
  {
    ERR << "Error while getting changed files for package " <<
    packageName << endl;
    return false;
  }

  if (fileList.size() <= 0)
  {
    DBG <<  "package " <<  packageName << " not changed -> no backup" << endl;
    return true;
  }

  if (filesystem::assert_dir(_root + _backuppath) != 0)
  {
    return false;
  }

  {
    // build up archive name
    time_t currentTime = time(0);
    struct tm *currentLocalTime = localtime(&currentTime);

    int date = (currentLocalTime->tm_year + 1900) * 10000
               + (currentLocalTime->tm_mon + 1) * 100
               + currentLocalTime->tm_mday;

    int num = 0;
    do
    {
      backupFilename = _root + _backuppath
                       + str::form("%s-%d-%d.tar.gz",packageName.c_str(), date, num);

    }
    while ( PathInfo(backupFilename).isExist() && num++ < 1000);

    PathInfo pi(filestobackupfile);
    if (pi.isExist() && !pi.isFile())
    {
      ERR << filestobackupfile.asString() << " already exists and is no file" << endl;
      return false;
    }

    std::ofstream fp ( filestobackupfile.asString().c_str(), std::ios::out|std::ios::trunc );

    if (!fp)
    {
      ERR << "could not open " << filestobackupfile.asString() << endl;
      return false;
    }

    for (FileList::const_iterator cit = fileList.begin();
         cit != fileList.end(); ++cit)
    {
      std::string name = *cit;
      if ( name[0] == '/' )
      {
        // remove slash, file must be relative to -C parameter of tar
        name = name.substr( 1 );
      }
      DBG << "saving file "<< name << endl;
      fp << name << endl;
    }
    fp.close();

    const char* const argv[] =
      {
        "tar",
        "-czhP",
        "-C",
        _root.asString().c_str(),
        "--ignore-failed-read",
        "-f",
        backupFilename.asString().c_str(),
        "-T",
        filestobackupfile.asString().c_str(),
        NULL
      };

    // execute tar in inst-sys (we dont know if there is a tar below _root !)
    ExternalProgram tar(argv, ExternalProgram::Stderr_To_Stdout, false, -1, true);

    std::string tarmsg;

    // TODO: it is probably possible to start tar with -v and watch it adding
    // files to report progress
    for (std::string output = tar.receiveLine(); output.length() ;output = tar.receiveLine())
    {
      tarmsg+=output;
    }

    int ret = tar.close();

    if ( ret != 0)
    {
      ERR << "tar failed: " << tarmsg << endl;
      ret = false;
    }
    else
    {
      MIL << "tar backup ok" << endl;
      progresslog.comment(
          str::form(_("created backup %s"), backupFilename.asString().c_str())
          , /*timestamp*/true);
    }

    filesystem::unlink(filestobackupfile);
  }

  return ret;
}

void RpmDb::setBackupPath(const Pathname& path)
{
  _backuppath = path;
}

std::ostream & operator<<( std::ostream & str, RpmDb::CheckPackageResult obj )
{
  switch ( obj )
  {
#define OUTS(E,S) case RpmDb::E: return str << "["<< (unsigned)obj << "-"<< S << "]"; break
    // translators: possible rpm package signature check result [brief]
    OUTS( CHK_OK,		_("Signature is OK") );
    // translators: possible rpm package signature check result [brief]
    OUTS( CHK_NOTFOUND,		_("Unknown type of signature") );
    // translators: possible rpm package signature check result [brief]
    OUTS( CHK_FAIL,		_("Signature does not verify") );
    // translators: possible rpm package signature check result [brief]
    OUTS( CHK_NOTTRUSTED,	_("Signature is OK, but key is not trusted") );
    // translators: possible rpm package signature check result [brief]
    OUTS( CHK_NOKEY,		_("Signatures public key is not available") );
    // translators: possible rpm package signature check result [brief]
    OUTS( CHK_ERROR,		_("File does not exist or signature can't be checked") );
    // translators: possible rpm package signature check result [brief]
    OUTS( CHK_NOSIG,		_("File is unsigned") );
#undef OUTS
  }
  return str << "UnknowSignatureCheckError("+str::numstring(obj)+")";
}

std::ostream & operator<<( std::ostream & str, const RpmDb::CheckPackageDetail & obj )
{
  for ( const auto & el : obj )
    str << el.second << endl;
  return str;
}

} // namespace rpm
} // namespace target
} // namespace zypp
