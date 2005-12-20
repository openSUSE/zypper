/*---------------------------------------------------------------------\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|                            Package Management                        |
|                                                     (C) 2002 SuSE AG |
\----------------------------------------------------------------------/

   File:       RpmDb.cc
   Purpose:    Interface to installed RPM system
   Author:     Stefan Schubert <schubi@suse.de>
   Maintainer: Ludwig Nussel <lnussel@suse.de>

   Copied and adapted from agent-targetpkg

/-*/
#include "librpm.h"

#include <cstdlib>
#include <cstdio>
#include <ctime>

#include <iostream>
#include <fstream>
#include <list>
#include <map>
#include <string>
#include <vector>

#include <y2util/Date.h>
#include <y2util/FSize.h>
#include <y2util/Vendor.h>
#include <y2util/Y2SLog.h>
#include <y2util/TagParser.h>
#include <y2util/Pathname.h>
#include <y2util/PathInfo.h>
#include <y2util/ExternalDataSource.h>
#include <y2util/diff.h>

#include <y2pm/RpmDb.h>
#include <y2pm/InstTargetError.h>
#include <y2pm/RpmDbCallbacks.h>

#include <y2pm/librpmDb.h>
#include <y2pm/PMRpmPackageDataProvider.h>
#include <y2pm/PMPackageManager.h>
#include <y2pm/Timecount.h>

#include <Y2PM.h>

#ifndef _
#define _(X) X
#endif

using namespace std;
using namespace RpmDbCallbacks;

/******************************************************************
**
**
**	FUNCTION NAME : stringPath
**	FUNCTION TYPE : inline string
*/
inline string stringPath( const Pathname & root_r, const Pathname & sub_r )
{
  return librpmDb::stringPath( root_r, sub_r );
}

/******************************************************************
**
**
**	FUNCTION NAME : operator<<
**	FUNCTION TYPE : ostream &
*/
ostream & operator<<( ostream & str, const RpmDb::DbStateInfoBits & obj )
{
  if ( obj == RpmDb::DbSI_NO_INIT ) {
    str << "NO_INIT";
  } else {
#define ENUM_OUT(B,C) str << ( obj & RpmDb::B ? C : '-' )
    str << "V4(";
    ENUM_OUT( DbSI_HAVE_V4,	'X' );
    ENUM_OUT( DbSI_MADE_V4,	'c' );
    ENUM_OUT( DbSI_MODIFIED_V4,	'm' );
    str << ")V3(";
    ENUM_OUT( DbSI_HAVE_V3,	'X' );
    ENUM_OUT( DbSI_HAVE_V3TOV4,	'B' );
    ENUM_OUT( DbSI_MADE_V3TOV4,	'c' );
    str << ")";
#undef ENUM_OUT
  }
  return str;
}

///////////////////////////////////////////////////////////////////
//	CLASS NAME : RpmDbPtr
//	CLASS NAME : RpmDbconstPtr
///////////////////////////////////////////////////////////////////
IMPL_BASE_POINTER(RpmDb);

#define WARNINGMAILPATH "/var/log/YaST2/"
#define FILEFORBACKUPFILES "YaSTBackupModifiedFiles"

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : RpmDb::Logfile
/**
 * Simple wrapper for progress log. Refcnt, filename and corresponding
 * ofstream are static members. Logfile constructor raises, destructor
 * lowers refcounter. On refcounter changing from 0->1, file is opened.
 * Changing from 1->0 the file is closed. Thus Logfile objects should be
 * local to those functions, writing the log, and must not be stored
 * permanently;
 *
 * Usage:
 *  some methothd ()
 *  {
 *    Logfile progresslog;
 *    ...
 *    progresslog() << "some message" << endl;
 *    ...
 *  }
 **/
class RpmDb::Logfile {
  Logfile( const Logfile & );
  Logfile & operator=( const Logfile & );
  private:
    static ofstream _log;
    static unsigned _refcnt;
    static Pathname _fname;
    static void openLog() {
      if ( !_fname.empty() ) {
	_log.clear();
	_log.open( _fname.asString().c_str(), std::ios::out|std::ios::app );
	if( !_log )
	  ERR << "Could not open logfile '" << _fname << "'" << endl;
      }
    }
    static void closeLog() {
      _log.clear();
      _log.close();
    }
    static void refUp() {
      if ( !_refcnt )
	openLog();
      ++_refcnt;
    }
    static void refDown() {
      --_refcnt;
      if ( !_refcnt )
	closeLog();
    }
  public:
    Logfile() { refUp(); }
    ~Logfile() { refDown(); }
    ostream & operator()( bool timestamp = false ) {
      if ( timestamp ) {
	_log << Date::form( "%Y-%m-%d %H:%M:%S ", Date::now() );
      }
      return _log;
    }
    static void setFname( const Pathname & fname_r ) {
      MIL << "installation log file " << fname_r << endl;
      if ( _refcnt )
	closeLog();
      _fname = fname_r;
      if ( _refcnt )
	openLog();
    }
};

///////////////////////////////////////////////////////////////////

Pathname RpmDb::Logfile::_fname;
ofstream RpmDb::Logfile::_log;
unsigned RpmDb::Logfile::_refcnt = 0;

///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::setInstallationLogfile
//	METHOD TYPE : bool
//
bool RpmDb::setInstallationLogfile( const Pathname & filename )
{
  Logfile::setFname( filename );
  return true;
}

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : RpmDb::Packages
/**
 * Helper class for RpmDb::getPackages() to build the
 * list<PMPackagePtr> returned. We have to assert, that there
 * is a unique entry for every PkgName.
 *
 * In the first step we build the _index map which helps to catch
 * multiple occurances of a PkgName in the rpmdb. That's not desired,
 * but possible. Usg. the last package instance installed is strored
 * in the _index map.
 *
 * At the end buildList() is called to build the list<PMPackagePtr>
 * from the _index map. _valid is set true to assign that the list
 * is in sync with the rpmdb content. Operations changing the rpmdb
 * content (install/remove package) should set _valid to false. The
 * next call to RpmDb::getPackages() will then reread the the rpmdb.
 *
 * Note that outside RpmDb::getPackages() _list and _index are always
 * in sync. So you may use lookup(PkgName) to retrieve a specific
 * PMPackagePtr.
 **/
class RpmDb::Packages {
  public:
    list<PMPackagePtr>        _list;
    map<PkgName,PMPackagePtr> _index;
    bool                      _valid;
    Packages() : _valid( false ) {}
    void clear() {
      _list.clear();
      _index.clear();
      _valid = false;
    }
    PMPackagePtr lookup( const PkgName & name_r ) const {
      map<PkgName,PMPackagePtr>::const_iterator got = _index.find( name_r );
      if ( got != _index.end() )
	return got->second;
      return PMPackagePtr();
    }
    void buildList() {
      _list.clear();
      for ( map<PkgName,PMPackagePtr>::iterator iter = _index.begin();
	    iter != _index.end(); ++iter ) {
	if ( iter->second )
	  _list.push_back( iter->second );
      }
      _valid = true;
    }
};

///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : RpmDb
//
///////////////////////////////////////////////////////////////////

#define FAILIFNOTINITIALIZED if( ! initialized() ) { WAR << "No database access: " << Error::E_RpmDB_not_open << endl; return Error::E_RpmDB_not_open; }

///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::RpmDb
//	METHOD TYPE : Constructor
//
RpmDb::RpmDb()
    : _dbStateInfo( DbSI_NO_INIT )
    , _packages( * new Packages ) // delete in destructor
#warning Check for obsolete memebers
    , _backuppath ("/var/adm/backup")
    , _packagebackups(false)
    , _warndirexists(false)
{
   process = 0;
   exit_code = -1;

   // Some rpm versions are patched not to abort installation if
   // symlink creation failed.
   setenv( "RPM_IgnoreFailedSymlinks", "1", 1 );
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::~RpmDb
//	METHOD TYPE : Destructor
//
RpmDb::~RpmDb()
{
   M__ << "~RpmDb()" << endl;
   closeDatabase();

   delete process;
   delete &_packages;
   M__  << "~RpmDb() end" << endl;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::dumpOn
//	METHOD TYPE : std::ostream &
//
std::ostream & RpmDb::dumpOn( std::ostream & str ) const
{
  str << "RpmDb[";

  if ( _dbStateInfo == DbSI_NO_INIT ) {
    str << "NO_INIT";
  } else {
#define ENUM_OUT(B,C) str << ( _dbStateInfo & B ? C : '-' )
    str << "V4(";
    ENUM_OUT( DbSI_HAVE_V4,	'X' );
    ENUM_OUT( DbSI_MADE_V4,	'c' );
    ENUM_OUT( DbSI_MODIFIED_V4,	'm' );
    str << ")V3(";
    ENUM_OUT( DbSI_HAVE_V3,	'X' );
    ENUM_OUT( DbSI_HAVE_V3TOV4,	'B' );
    ENUM_OUT( DbSI_MADE_V3TOV4,	'c' );
    str << "): " << stringPath( _root, _dbPath );
#undef ENUM_OUT
  }
  return str << "]";
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::initDatabase
//	METHOD TYPE : PMError
//
PMError RpmDb::initDatabase( Pathname root_r, Pathname dbPath_r )
{
  ///////////////////////////////////////////////////////////////////
  // Check arguments
  ///////////////////////////////////////////////////////////////////
  if ( root_r.empty() )
    root_r = "/";

  if ( dbPath_r.empty() )
    dbPath_r = "/var/lib/rpm";

  if ( ! (root_r.absolute() && dbPath_r.absolute()) ) {
    ERR << "Illegal root or dbPath: " << stringPath( root_r, dbPath_r ) << endl;
    return Error::E_invalid_argument;
  }

  MIL << "Calling initDatabase: " << stringPath( root_r, dbPath_r ) << endl;

  ///////////////////////////////////////////////////////////////////
  // Check whether already initialized
  ///////////////////////////////////////////////////////////////////
  if ( initialized() ) {
    if ( root_r == _root && dbPath_r == _dbPath ) {
      return Error::E_ok;
    } else {
      ERR << "Can't switch to " << stringPath( root_r, dbPath_r )
	<< " while accessing " << stringPath( _root, _dbPath ) << endl;
      return Error::E_RpmDB_already_open;
    }
  }

  ///////////////////////////////////////////////////////////////////
  // init database
  ///////////////////////////////////////////////////////////////////
  librpmDb::unblockAccess();
  DbStateInfoBits info = DbSI_NO_INIT;
  PMError err = internal_initDatabase( root_r, dbPath_r, info );

  if ( err ) {
    librpmDb::blockAccess();
    ERR << "Cleanup on error: state " << info << endl;

    if ( dbsi_has( info, DbSI_MADE_V4 ) ) {
      // remove the newly created rpm4 database and
      // any backup created on conversion.
      removeV4( root_r + dbPath_r, dbsi_has( info, DbSI_MADE_V3TOV4 ) );
    }

  } else {
    if ( dbsi_has( info, DbSI_HAVE_V3 ) ) {
      if ( root_r == "/" || dbsi_has( info, DbSI_MODIFIED_V4 ) ) {
	// Move obsolete rpm3 database beside.
	MIL << "Cleanup: state " << info << endl;
	removeV3( root_r + dbPath_r, dbsi_has( info, DbSI_MADE_V3TOV4 ) );
	dbsi_clr( info, DbSI_HAVE_V3 );
      } else {
	// Performing an update: Keep the original rpm3 database
	// and wait if the rpm4 database gets modified by installing
	// or removing packages. Cleanup in modifyDatabase or closeDatabase.
	MIL << "Update mode: Cleanup delayed until closeOldDatabase." << endl;
      }
    }
#warning CHECK: notify root about conversion backup.

    _root   = root_r;
    _dbPath = dbPath_r;
    _dbStateInfo = info;

    if ( ! ( err || Y2PM::runningFromSystem() ) ) {
      if (      dbsi_has( info, DbSI_HAVE_V4 )
	   && ! dbsi_has( info, DbSI_MADE_V4 ) ) {
	err = rebuildDatabase();
      }
    }

    // Close the database in case any write acces (create/convert)
    // happened during init. This should drop any lock acquired
    // by librpm. On demand it will be reopened readonly and should
    // not hold any lock.
    librpmDb::dbRelease( true );
    MIL << "InitDatabase: " << *this << endl;
  }

  return err;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::internal_initDatabase
//	METHOD TYPE : PMError
//
PMError RpmDb::internal_initDatabase( const Pathname & root_r, const Pathname & dbPath_r,
				      DbStateInfoBits & info_r )
{
  info_r = DbSI_NO_INIT;

  ///////////////////////////////////////////////////////////////////
  // Get info about the desired database dir
  ///////////////////////////////////////////////////////////////////
  librpmDb::DbDirInfo dbInfo( root_r, dbPath_r );

  if ( dbInfo.illegalArgs() ) {
    return Error::E_invalid_argument; // should not happen (checked in initDatabase)
  }
  if ( ! dbInfo.usableArgs() ) {
    ERR << "Bad database directory: " << dbInfo.dbDir() << endl;
    return Error::E_invalid_argument;
  }

  if ( dbInfo.hasDbV4() ) {
    dbsi_set( info_r, DbSI_HAVE_V4 );
    MIL << "Found rpm4 database in " << dbInfo.dbDir() << endl;
  } else {
    MIL << "Creating new rpm4 database in " << dbInfo.dbDir() << endl;
  }

  if ( dbInfo.hasDbV3() ) {
    dbsi_set( info_r, DbSI_HAVE_V3 );
  }
  if ( dbInfo.hasDbV3ToV4() ) {
    dbsi_set( info_r, DbSI_HAVE_V3TOV4 );
  }

  DBG << "Initial state: " << info_r << ": " << stringPath( root_r, dbPath_r );
  librpmDb::dumpState( DBG ) << endl;

  ///////////////////////////////////////////////////////////////////
  // Access database, create if needed
  ///////////////////////////////////////////////////////////////////

  // creates dbdir and empty rpm4 database if not present
  PMError err = librpmDb::dbAccess( root_r, dbPath_r );

  if ( ! dbInfo.hasDbV4() ) {
    dbInfo.restat();
    if ( dbInfo.hasDbV4() ) {
      dbsi_set( info_r, DbSI_HAVE_V4 | DbSI_MADE_V4 );
    }
  }

  if ( err ) {
    ERR << "Can't access rpm4 database " << dbInfo.dbV4() << " " << err << endl;
    return err;
  }

  DBG << "Access state: " << info_r << ": " << stringPath( root_r, dbPath_r );
  librpmDb::dumpState( DBG ) << endl;

  ///////////////////////////////////////////////////////////////////
  // Check whether to convert something. Create backup but do
  // not remove anything here
  ///////////////////////////////////////////////////////////////////
  constlibrpmDbPtr dbptr;
  librpmDb::dbAccess( dbptr );
  bool dbEmpty = dbptr->empty();
  if ( dbEmpty ) {
    MIL << "Empty rpm4 database "  << dbInfo.dbV4() << endl;
  }

  if ( dbInfo.hasDbV3() ) {
    MIL << "Found rpm3 database " << dbInfo.dbV3() << endl;

    if ( dbEmpty ) {
      extern PMError convertV3toV4( const Pathname & v3db_r, const constlibrpmDbPtr & v4db_r );

      err = convertV3toV4( dbInfo.dbV3().path(), dbptr );
      if ( err ) {
	return err;
      }

      // create a backup copy
      int res = PathInfo::copy( dbInfo.dbV3().path(), dbInfo.dbV3ToV4().path() );
      if ( res ) {
	WAR << "Backup converted rpm3 database failed: error(" << res << ")" << endl;
      } else {
	dbInfo.restat();
	if ( dbInfo.hasDbV3ToV4() ) {
	  MIL << "Backup converted rpm3 database: " << dbInfo.dbV3ToV4() << endl;
	  dbsi_set( info_r, DbSI_HAVE_V3TOV4 | DbSI_MADE_V3TOV4 );
	}
      }

    } else {

      WAR << "Non empty rpm3 and rpm4 database found: using rpm4" << endl;
#warning EXCEPTION: nonempty rpm4 and rpm3 database found.
      //ConvertDbReport::Send report( RpmDbCallbacks::convertDbReport );
      //report->start( dbInfo.dbV3().path() );
      //report->stop( some error );

      // set DbSI_MODIFIED_V4 as it's not a temporary which can be removed.
      dbsi_set( info_r, DbSI_MODIFIED_V4 );

    }

    DBG << "Convert state: " << info_r << ": " << stringPath( root_r, dbPath_r );
    librpmDb::dumpState( DBG ) << endl;
  }

  if ( dbInfo.hasDbV3ToV4() ) {
    MIL << "Rpm3 database backup: " << dbInfo.dbV3ToV4() << endl;
  }

  return err;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::removeV4
//	METHOD TYPE : void
//
void RpmDb::removeV4( const Pathname & dbdir_r, bool v3backup_r )
{
  const char * v3backup = "packages.rpm3";
  const char * master = "Packages";
  const char * index[] = {
    "Basenames",
    "Conflictname",
    "Depends",
    "Dirnames",
    "Filemd5s",
    "Group",
    "Installtid",
    "Name",
    "Providename",
    "Provideversion",
    "Pubkeys",
    "Requirename",
    "Requireversion",
    "Sha1header",
    "Sigmd5",
    "Triggername",
    // last entry!
    NULL
  };

  PathInfo pi( dbdir_r );
  if ( ! pi.isDir() ) {
    ERR << "Can't remove rpm4 database in non directory: " << dbdir_r << endl;
    return;
  }

  for ( const char ** f = index; *f; ++f ) {
    pi( dbdir_r + *f );
    if ( pi.isFile() ) {
      PathInfo::unlink( pi.path() );
    }
  }

  pi( dbdir_r + master );
  if ( pi.isFile() ) {
    MIL << "Removing rpm4 database " << pi << endl;
    PathInfo::unlink( pi.path() );
  }

  if ( v3backup_r ) {
    pi( dbdir_r + v3backup );
    if ( pi.isFile() ) {
      MIL << "Removing converted rpm3 database backup " << pi << endl;
      PathInfo::unlink( pi.path() );
    }
  }
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::removeV3
//	METHOD TYPE : void
//
void RpmDb::removeV3( const Pathname & dbdir_r, bool v3backup_r )
{
  const char * master = "packages.rpm";
  const char * index[] = {
    "conflictsindex.rpm",
    "fileindex.rpm",
    "groupindex.rpm",
    "nameindex.rpm",
    "providesindex.rpm",
    "requiredby.rpm",
    "triggerindex.rpm",
    // last entry!
    NULL
  };

  PathInfo pi( dbdir_r );
  if ( ! pi.isDir() ) {
    ERR << "Can't remove rpm3 database in non directory: " << dbdir_r << endl;
    return;
  }

  for ( const char ** f = index; *f; ++f ) {
    pi( dbdir_r + *f );
    if ( pi.isFile() ) {
      PathInfo::unlink( pi.path() );
    }
  }

#warning CHECK: compare vs existing v3 backup. notify root
  pi( dbdir_r + master );
  if ( pi.isFile() ) {
    Pathname m( pi.path() );
    if ( v3backup_r ) {
      // backup was already created
      PathInfo::unlink( m );
      Pathname b( m.extend( "3" ) );
      pi( b ); // stat backup
    } else {
      Pathname b( m.extend( ".deleted" ) );
      pi( b );
      if ( pi.isFile() ) {
	// rempve existing backup
	PathInfo::unlink( b );
      }
      PathInfo::rename( m, b );
      pi( b ); // stat backup
    }
    MIL << "(Re)moved rpm3 database to " << pi << endl;
  }
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::modifyDatabase
//	METHOD TYPE : void
//
void RpmDb::modifyDatabase()
{
  if ( ! initialized() )
    return;

  // tag database as modified
  dbsi_set( _dbStateInfo, DbSI_MODIFIED_V4 );

  // Move outdated rpm3 database beside.
  if ( dbsi_has( _dbStateInfo, DbSI_HAVE_V3 ) ) {
    MIL << "Update mode: Delayed cleanup: state " << _dbStateInfo << endl;
    removeV3( _root + _dbPath, dbsi_has( _dbStateInfo, DbSI_MADE_V3TOV4 ) );
    dbsi_clr( _dbStateInfo, DbSI_HAVE_V3 );
  }

  // invalidate Packages list
  _packages._valid = false;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::closeDatabase
//	METHOD TYPE : PMError
//
PMError RpmDb::closeDatabase()
{
  if ( ! initialized() ) {
    return Error::E_ok;
  }

  MIL << "Calling closeDatabase: " << *this << endl;

  ///////////////////////////////////////////////////////////////////
  // Block further database access
  ///////////////////////////////////////////////////////////////////
  _packages.clear();
  librpmDb::blockAccess();

  ///////////////////////////////////////////////////////////////////
  // Check fate if old version database still present
  ///////////////////////////////////////////////////////////////////
  if ( dbsi_has( _dbStateInfo, DbSI_HAVE_V3 ) ) {
    MIL << "Update mode: Delayed cleanup: state " << _dbStateInfo << endl;
    if ( dbsi_has( _dbStateInfo, DbSI_MODIFIED_V4 ) ) {
      // Move outdated rpm3 database beside.
      removeV3( _root + _dbPath, dbsi_has( _dbStateInfo, DbSI_MADE_V3TOV4 )  );
    } else {
      // Remove unmodified rpm4 database
      removeV4( _root + _dbPath, dbsi_has( _dbStateInfo, DbSI_MADE_V3TOV4 ) );
    }
  }

  ///////////////////////////////////////////////////////////////////
  // Uninit
  ///////////////////////////////////////////////////////////////////
  _root = _dbPath = Pathname();
  _dbStateInfo = DbSI_NO_INIT;

  MIL << "closeDatabase: " << *this << endl;
  return Error::E_ok;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::rebuildDatabase
//	METHOD TYPE : PMError
//
PMError RpmDb::rebuildDatabase()
{
  FAILIFNOTINITIALIZED;

  MIL << "RpmDb::rebuildDatabase" << *this << endl;
  Timecount _t( "RpmDb::rebuildDatabase" );

  PathInfo dbMaster( root() + dbPath() + "Packages" );
  PathInfo dbMasterBackup( dbMaster.path().extend( ".y2backup" ) );

  // report
  RebuildDbReport::Send report( rebuildDbReport );
  report->start();
  ProgressData pd;
  // current size should be upper limit for new db
  report->progress( pd.init( dbMaster.size() ) );

  // run rpm
  RpmArgVec opts;
  opts.push_back("--rebuilddb");
  opts.push_back("-vv");

  // don't call modifyDatabase because it would remove the old
  // rpm3 database, if the current database is a temporary one.
  // But do invalidate packages list.
  _packages._valid = false;
  run_rpm (opts, ExternalProgram::Stderr_To_Stdout);

  // progress report: watch this file growing
  PathInfo newMaster( root()
		      + dbPath().extend( stringutil::form( "rebuilddb.%d",
							   process?process->getpid():0) )
		      + "Packages" );

  string       line;
  string       errmsg;

  while ( systemReadLine( line ) ) {
    if ( newMaster() ) { // file is removed at the end of rebuild.
      report->progress( pd.set( newMaster.size() ) );
    }

    if ( line.compare( 0, 2, "D:" ) ) {
      errmsg += line + '\n';
      report->notify( line );
      WAR << line << endl;
    }
  }

  int rpm_status = systemStatus();

  // evaluate result
  PMError err;

  if ( rpm_status != 0 ) {
    ERR << "rpm failed, message was:" << endl << errmsg; // has trailing NL
    err =  Error::E_RpmDB_subprocess_failed;
    err.setDetails(errmsg);
  } else {
    report->progress( pd.toMax() ); // 100%
  }

  report->stop( err );
  return err;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::importPubkey
//	METHOD TYPE : PMError
//
PMError RpmDb::importPubkey( const Pathname & pubkey_r )
{
  FAILIFNOTINITIALIZED;
  PMError err;

  RpmArgVec opts;
  opts.push_back ( "--import" );
  opts.push_back ( "--" );
  opts.push_back ( pubkey_r.asString().c_str() );

  // don't call modifyDatabase because it would remove the old
  // rpm3 database, if the current database is a temporary one.
  // But do invalidate packages list.
  _packages._valid = false;
  run_rpm( opts, ExternalProgram::Stderr_To_Stdout );

  string line;
  while ( systemReadLine( line ) ) {
    if ( line.substr( 0, 6 ) == "error:" ) {
      WAR << line << endl;
    } else {
      DBG << line << endl;
    }
  }

  int rpm_status = systemStatus();

  if ( rpm_status != 0 ) {
    ERR << "Failed to import public key from file " << pubkey_r << ": rpm returned  " << rpm_status << endl;
    err = Error::E_RpmDB_subprocess_failed;
  } else {
    MIL << "Imported public key from file " << pubkey_r << endl;
  }

  return err;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::importPubkey
//	METHOD TYPE : PMError
//
PMError RpmDb::importPubkey( const Pathname & keyring_r, const string & keyname_r )
{
  FAILIFNOTINITIALIZED;

  // create tempfile
  char tmpname[] = "/tmp/y2.pubkey.XXXXXX";
  int tmpfd = mkstemp( tmpname );
  if ( tmpfd == -1 ) {
    ERR << "Unable to create a unique temporary file for pubkey" << endl;
    return Error::E_RpmDB_subprocess_failed;
  }

  // export keyname from keyring
  RpmArgVec args;
  args.push_back( "gpg" );
  args.push_back( "--armor" );
  args.push_back( "--no-default-keyring" );
  args.push_back( "--keyring" );
  args.push_back( keyring_r.asString().c_str() );
  args.push_back( "--export" );
  args.push_back( keyname_r.c_str() );

  const char * argv[args.size() + 1];
  const char ** p = argv;
  p = copy( args.begin(), args.end(), p );
  *p = 0;

  // launch gpg
  ExternalProgram prg( argv, ExternalProgram::Discard_Stderr, false, -1, true );
  PMError err;
  int res = 0;

  // read key
  for ( string line( prg.receiveLine() ); line.length(); line = prg.receiveLine() ) {
    ssize_t written = write( tmpfd, line.c_str(), line.length() );
    if ( written == -1 || unsigned(written) != line.length() ) {
      ERR << "Error writing pubkey to " << tmpname << endl;
      err = Error::E_RpmDB_subprocess_failed;
      break;
    }
    res += written; // empty file indicates key not found
  }
  close( tmpfd );

  if ( ! res ) {
    WAR << "gpg: no key '" << keyname_r << "' found in  '" << keyring_r << "'" << endl;
    err = Error::E_RpmDB_subprocess_failed;
  }

  // check gpg returncode
  res = prg.close();
  if ( res ) {
    ERR << "gpg: export '" << keyname_r << "' from '" << keyring_r << "' returned " << res << endl;
    err = Error::E_RpmDB_subprocess_failed;
  }

  if ( ! err ) {
    MIL << "Exported '" << keyname_r << "' from '" << keyring_r << "' to " << tmpname << endl;
    err = importPubkey( tmpname );
  }

  // remove tempfile
  PathInfo::unlink( tmpname );

  return err;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::pubkeys
//	METHOD TYPE : set<PkgEdition>
//
set<PkgEdition> RpmDb::pubkeys() const
{
  set<PkgEdition> ret;

  librpmDb::db_const_iterator it;
  for ( it.findByName( PkgName( "gpg-pubkey" ) ); *it; ++it ) {
    ret.insert( it->tag_edition() );
  }

  return ret;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::packagesValid
//	METHOD TYPE : bool
//
bool RpmDb::packagesValid() const
{
  return( _packages._valid || ! initialized() );
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::getPackages
//	METHOD TYPE : const std::list<PMPackagePtr> &
//
//	DESCRIPTION :
//
const std::list<PMPackagePtr> & RpmDb::getPackages()
{
  if ( packagesValid() ) {
    return _packages._list;
  }

  // report
  Timecount _t( "RpmDb::getPackages" );
  ScanDbReport::Send report( scanDbReport );
  report->start();

#warning how to detect corrupt db while reading.

  _packages.clear();

  ///////////////////////////////////////////////////////////////////
  // Collect package data. A map is used to check whethere there are
  // multiple entries for the same PkgName. If so we consider the last
  // one installed to be the one we're interesed in.
  ///////////////////////////////////////////////////////////////////
  ProgressData pd;
  librpmDb::db_const_iterator iter; // findAll
  {
    // quick check
    unsigned expect = 0;
    for ( ; *iter; ++iter ) {
      ++expect;
    }
    if ( iter.dbError() ) {
      ERR << "No database access: " << iter.dbError() << endl;
      report->stop( iter.dbError() );
      return _packages._list;
    }
    report->progress( pd.init( expect ) );
  }

  for ( iter.findAll(); *iter; ++iter, report->progress( pd.incr() ) ) {

    PkgName name = iter->tag_name();
    if ( name == PkgName( "gpg-pubkey" ) ) {
      // pseudo package filtered, as we can't handle multiple instances
      // of 'gpg-pubkey-VERS-REL'.
      continue;
    }
    Date installtime = iter->tag_installtime();
    PMPackagePtr & nptr = _packages._index[name]; // be shure to get a reference!

    if ( nptr ) {
      WAR << "Multiple entries for package '" << name << "' in rpmdb" << endl;
      if ( nptr->installtime() > installtime )
	continue;
      // else overwrite previous entry
    }

    // create dataprovider and package
    PMRpmPackageDataProviderPtr ndp = new PMRpmPackageDataProvider( this );
    nptr = new PMPackage( name, iter->tag_edition(), iter->tag_arch(), ndp );

    // add PMSolvable data to package, collect filerequires on the fly
    nptr->setProvides ( iter->tag_provides ( &_filerequires ) );
    nptr->setRequires ( iter->tag_requires ( &_filerequires ) );
    nptr->setConflicts( iter->tag_conflicts( &_filerequires ) );
    nptr->setObsoletes( iter->tag_obsoletes( &_filerequires ) );

    // let dataprovider collect static data
    ndp->loadStaticData( *iter );
  }

  ///////////////////////////////////////////////////////////////////
  // Evaluate filerequires collected so far
  ///////////////////////////////////////////////////////////////////
  for( FileDeps::FileNames::iterator it = _filerequires.begin(); it != _filerequires.end(); ++it ) {

    for ( iter.findByFile( *it ); *iter; ++iter ) {
      PMPackagePtr pptr = _packages.lookup( iter->tag_name() );
      if ( !pptr ) {
	WAR << "rpmdb.findByFile returned unpknown package " << *iter << endl;
	continue;
      }
      pptr->addProvides( *it );
    }

  }

  ///////////////////////////////////////////////////////////////////
  // Build final packages list
  ///////////////////////////////////////////////////////////////////
  _packages.buildList();
  DBG << "Found installed packages: " << _packages._list.size() << endl;
  report->stop( PMError::E_ok );
  return _packages._list;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::traceFileRel
//	METHOD TYPE : void
//
//	DESCRIPTION :
//
void RpmDb::traceFileRel( const PkgRelation & rel_r )
{
  if ( ! rel_r.isFileRel() )
    return;

  if ( ! _filerequires.insert( rel_r.name() ).second )
    return; // already got it in _filerequires

  if ( ! _packages._valid )
    return; // collect only. Evaluated in first call to getPackages()

  //
  // packages already initialized. Must check and insert here
  //
  librpmDb::db_const_iterator iter;
  if ( iter.dbError() ) {
    ERR << "No database access: " << iter.dbError() << endl;
    return;
  }

  for ( iter.findByFile( rel_r.name() ); *iter; ++iter ) {
    PMPackagePtr pptr = _packages.lookup( iter->tag_name() );
    if ( !pptr ) {
      WAR << "rpmdb.findByFile returned unpknown package " << *iter << endl;
      continue;
    }
    pptr->addProvides( rel_r.name() );
  }
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::hasFile
//	METHOD TYPE : bool
//
//	DESCRIPTION :
//
bool RpmDb::hasFile( const std::string & file_r ) const
{
  librpmDb::db_const_iterator it;
  return it.findByFile( file_r );
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
bool RpmDb::hasPackage( const PkgName & name_r ) const
{
  librpmDb::db_const_iterator it;
  return it.findPackage( name_r );
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::getData
//	METHOD TYPE : PMError
//
//	DESCRIPTION :
//
PMError RpmDb::getData( const PkgName & name_r,
			constRpmHeaderPtr & result_r ) const
{
  librpmDb::db_const_iterator it;
  it.findPackage( name_r );
  result_r = *it;
  return it.dbError();
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::getData
//	METHOD TYPE : PMError
//
//	DESCRIPTION :
//
PMError RpmDb::getData( const PkgName & name_r, const PkgEdition & ed_r,
			constRpmHeaderPtr & result_r ) const
{
  librpmDb::db_const_iterator it;
  it.findPackage( name_r, ed_r  );
  result_r = *it;
  return it.dbError();
}

/*--------------------------------------------------------------*/
/* Checking the source rpm <rpmpath> with rpm --chcksig and     */
/* the version number.						*/
/*--------------------------------------------------------------*/
unsigned
RpmDb::checkPackage (const Pathname & packagePath, string version, string md5 )
{
    unsigned result = 0;

    if ( ! version.empty() ) {
      constRpmHeaderPtr h( RpmHeader::readPackage( packagePath, RpmHeader::NOSIGNATURE ) );
      if ( ! h || PkgEdition( version ) != h->tag_edition() ) {
	result |= CHK_INCORRECT_VERSION;
      }
    }

    if(!md5.empty())
    {
#warning TBD MD5 check
	WAR << "md5sum check not yet implemented" << endl;
	return CHK_INCORRECT_FILEMD5;
    }

    std::string path = packagePath.asString();
    // checking --checksig
    const char *const argv[] = {
	"rpm", "--checksig", "--", path.c_str(), 0
    };

    exit_code = -1;

    string output = "";
    unsigned int k;
    for ( k = 0; k < (sizeof(argv) / sizeof(*argv)) -1; k++ )
    {
	output = output + " " + argv[k];
    }

    DBG << "rpm command: " << output << endl;

    if ( process != NULL )
    {
	delete process;
	process = NULL;
    }
    // Launch the program
    process = new ExternalProgram( argv, ExternalProgram::Stderr_To_Stdout, false, -1, true);


    if ( process == NULL )
    {
	result |= CHK_OTHER_FAILURE;
	D__ << "create process failed" << endl;
    }

    string value;
    output = process->receiveLine();

    while ( output.length() > 0)
    {
	string::size_type         ret;

	// extract \n
	ret = output.find_first_of ( "\n" );
	if ( ret != string::npos )
	{
	    value.assign ( output, 0, ret );
	}
	else
	{
	    value = output;
	}

	D__ << "stdout: " << value << endl;

	string::size_type pos;
	if((pos = value.find (path)) != string::npos)
	{
	    string rest = value.substr (pos + path.length() + 1);
	    if (rest.find("NOT OK") == string::npos)
	    {
		// see what checks are ok
		if (rest.find("md5") == string::npos)
		{
		    result |= CHK_MD5SUM_MISSING;
		}
		if (rest.find("gpg") == string::npos)
		{
		    result |= CHK_GPGSIG_MISSING;
		}
	    }
	    else
	    {
		// see what checks are not ok
		if (rest.find("MD5") != string::npos)
		{
		    result |= CHK_INCORRECT_PKGMD5;
		}
		else
		{
		    result |= CHK_MD5SUM_MISSING;
		}

		if (rest.find("GPG") != string::npos)
		{
		    result |= CHK_INCORRECT_GPGSIG;
		}
		else
		{
		    result |= CHK_GPGSIG_MISSING;
		}
	    }
	}

	output = process->receiveLine();
    }

    if ( result == 0 && systemStatus() != 0 )
    {
	// error
	result |= CHK_OTHER_FAILURE;
    }

    return ( result );
}

// determine changed files of installed package
bool
RpmDb::queryChangedFiles(FileList & fileList, const string& packageName)
{
    bool ok = true;

    fileList.clear();

    if( ! initialized() ) return false;

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

    string line;
    while (systemReadLine(line))
    {
	if (line.length() > 12 &&
	    (line[0] == 'S' || line[0] == 's' ||
	     (line[0] == '.' && line[7] == 'T')))
	{
	    // file has been changed
	    string filename;

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
    if ( process ) {
	delete process;
	process = NULL;
    }
    exit_code = -1;

    if ( ! initialized() ) {
	ERR << "Attempt to run rpm: " << Error::E_RpmDB_not_open << endl;
	return;
    }

    RpmArgVec args;

    // always set root and dbpath
    args.push_back("rpm");
    args.push_back("--root");
    args.push_back(_root.asString().c_str());
    args.push_back("--dbpath");
    args.push_back(_dbPath.asString().c_str());

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
bool
RpmDb::systemReadLine(string &line)
{
    line.erase();

    if ( process == NULL )
	return false;

    line = process->receiveLine();

    if (line.length() == 0)
	return false;

    if (line[line.length() - 1] == '\n')
	line.erase(line.length() - 1);

    return true;
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
   process->kill();
   delete process;
   process = 0;

//   D__ << "exit code " << exit_code << endl;

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
void RpmDb::processConfigFiles(const string& line, const string& name, const char* typemsg, const char* difffailmsg, const char* diffgenmsg)
{
    string msg = line.substr(9);
    string::size_type pos1 = string::npos;
    string::size_type pos2 = string::npos;
    string file1s, file2s;
    Pathname file1;
    Pathname file2;

    pos1 = msg.find (typemsg);
    for (;;)
    {
	if( pos1 == string::npos )
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

	string out;
	int ret = Diff::differ (file1.asString(), file2.asString(), out, 25);
	if (ret)
	{
	    Pathname file = _root + WARNINGMAILPATH;
	    if (PathInfo::assert_dir(file) != 0)
	    {
		ERR << "Could not create " << file.asString() << endl;
		break;
	    }
	    file += Date::form("config_diff_%Y_%m_%d.log", Date::now());
	    ofstream notify(file.asString().c_str(), std::ios::out|std::ios::app);
	    if(!notify)
	    {
		ERR << "Could not open " <<  file << endl;
		break;
	    }

	    // Translator: %s = name of an rpm package. A list of diffs follows
	    // this message.
	    notify << stringutil::form(_("Changed configuration files for %s:"), name.c_str()) << endl;
	    if(ret>1)
	    {
		ERR << "diff failed" << endl;
		notify << stringutil::form(difffailmsg,
		    file1s.c_str(), file2s.c_str()) << endl;
	    }
	    else
	    {
		notify << stringutil::form(diffgenmsg,
		    file1s.c_str(), file2s.c_str()) << endl;

		// remove root for the viewer's pleasure (#38240)
		if (!_root.empty() && _root != "/")
		{
		    if(out.substr(0,4) == "--- ")
		    {
			out.replace(4, file1.asString().length(), file1s);
		    }
		    string::size_type pos = out.find("\n+++ ");
		    if(pos != string::npos)
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
PMError RpmDb::installPackage( const Pathname & filename, unsigned flags )
{
    FAILIFNOTINITIALIZED;
    Logfile progresslog;

    MIL << "RpmDb::installPackage(" << filename << "," << flags << ")" << endl;

    // report
    InstallPkgReport::Send report( installPkgReport );
    report->start( filename );
    ProgressData pd;

    // backup
    if ( _packagebackups ) {
      report->progress( pd.init( -2, 100 ) ); // allow 1% for backup creation.
      if ( ! backupPackage( filename ) ) {
	ERR << "backup of " << filename.asString() << " failed" << endl;
      }
      report->progress( pd.set( 0 ) ); // allow 1% for backup creation.
    } else {
      report->progress( pd.init( 100 ) );
    }

    // run rpm
    RpmArgVec opts;
    if (flags & RPMINST_NOUPGRADE)
      opts.push_back("-i");
    else
      opts.push_back("-U");
    opts.push_back("--percent");

    if (flags & RPMINST_NODIGEST)
	opts.push_back("--nodigest");
    if (flags & RPMINST_NOSIGNATURE)
	opts.push_back("--nosignature");
    if (flags & RPMINST_NODOCS)
	opts.push_back ("--excludedocs");
    if (flags & RPMINST_NOSCRIPTS)
	opts.push_back ("--noscripts");
    if (flags & RPMINST_FORCE)
	opts.push_back ("--force");
    if (flags & RPMINST_NODEPS)
	opts.push_back ("--nodeps");
    if(flags & RPMINST_IGNORESIZE)
	opts.push_back ("--ignoresize");
    if(flags & RPMINST_JUSTDB)
	opts.push_back ("--justdb");

    opts.push_back("--");
    opts.push_back (filename.asString().c_str());

    modifyDatabase(); // BEFORE run_rpm
    run_rpm( opts, ExternalProgram::Stderr_To_Stdout );

    string line;
    string rpmmsg;
    vector<string> configwarnings;
    vector<string> errorlines;

    while (systemReadLine(line))
    {
	if (line.substr(0,2)=="%%")
	{
	    int percent;
	    sscanf (line.c_str () + 2, "%d", &percent);
	    report->progress( pd.set( percent ) );
	}
	else
	    rpmmsg += line+'\n';

	if( line.substr(0,8) == "warning:" )
	{
	    configwarnings.push_back(line);
	}
    }
    int rpm_status = systemStatus();

    // evaluate result
    for(vector<string>::iterator it = configwarnings.begin();
	it != configwarnings.end(); ++it)
    {
	    processConfigFiles(*it, Pathname::basename(filename), " saved as ",
		// %s = filenames
		_("rpm saved %s as %s but it was impossible to determine the difference"),
		// %s = filenames
		_("rpm saved %s as %s.\nHere are the first 25 lines of difference:\n"));
	    processConfigFiles(*it, Pathname::basename(filename), " created as ",
		// %s = filenames
		_("rpm created %s as %s but it was impossible to determine the difference"),
		// %s = filenames
		_("rpm created %s as %s.\nHere are the first 25 lines of difference:\n"));
    }

    PMError err;

    if ( rpm_status != 0 )  {
      // %s = filename of rpm package
      progresslog(/*timestamp*/true) << stringutil::form(_("%s install failed"), Pathname::basename(filename).c_str()) << endl;
      progresslog() << _("rpm output:") << endl << rpmmsg << endl;
      ERR << "rpm failed, message was: " << rpmmsg << endl;
      err = Error::E_RpmDB_subprocess_failed;
      err.setDetails( rpmmsg );
    } else {
      // %s = filename of rpm package
      progresslog(/*timestamp*/true) << stringutil::form(_("%s installed ok"), Pathname::basename(filename).c_str()) << endl;
      if( ! rpmmsg.empty() ) {
	progresslog() << _("Additional rpm output:") << endl << rpmmsg << endl;
      }
    }

    report->stop( err );
    return err;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::removePackage
//	METHOD TYPE : PMError
//
PMError RpmDb::removePackage( constPMPackagePtr package, unsigned flags )
{
  return removePackage( package->name(), flags );
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::removePackage
//	METHOD TYPE : PMError
//
PMError RpmDb::removePackage( const string & name_r, unsigned flags )
{
    FAILIFNOTINITIALIZED;
    Logfile progresslog;

    MIL << "RpmDb::removePackage(" << name_r << "," << flags << ")" << endl;

    // report
    RemovePkgReport::Send report( removePkgReport );
    report->start( name_r );
    ProgressData pd;

    // backup
    if ( _packagebackups ) {
      report->progress( pd.init( -2, 100 ) ); // allow 1% for backup creation.
      if ( ! backupPackage( name_r ) ) {
	ERR << "backup of " << name_r << " failed" << endl;
      }
      report->progress( pd.set( 0 ) );
    } else {
      report->progress( pd.init( 100 ) );
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
    if (flags & RPMINST_FORCE) {
      WAR << "IGNORE OPTION: 'rpm -e' does not support '--force'" << endl;
    }

    opts.push_back("--");
    opts.push_back(name_r.c_str());

    modifyDatabase(); // BEFORE run_rpm
    run_rpm (opts, ExternalProgram::Stderr_To_Stdout);

    string line;
    string rpmmsg;

    // got no progress from command, so we fake it:
    // 5  - command started
    // 50 - command completed
    // 100 if no error
    report->progress( pd.set( 5 ) );
    while (systemReadLine(line))
    {
	rpmmsg += line+'\n';
    }
    report->progress( pd.set( 50 ) );
    int rpm_status = systemStatus();

    // evaluate result
    PMError err;

    if ( rpm_status != 0 ) {
      // %s = name of rpm package
      progresslog(/*timestamp*/true) << stringutil::form(_("%s remove failed"), name_r.c_str()) << endl;
      progresslog() << _("rpm output:") << endl << rpmmsg << endl;
      ERR << "rpm failed, message was: " << rpmmsg << endl;
      err =  Error::E_RpmDB_subprocess_failed;
      err.setDetails( rpmmsg );
    } else {
      progresslog(/*timestamp*/true) << stringutil::form(_("%s remove ok"), name_r.c_str()) << endl;
      if( ! rpmmsg.empty() ) {
	progresslog() << _("Additional rpm output:") << endl << rpmmsg << endl;
      }
    }

    report->stop( err );
    return err;
}

string
RpmDb::checkPackageResult2string(unsigned code)
{
    string msg;
    // begin of line characters
    string bol = " - ";
    // end of line characters
    string eol = "\n";
    if(code == 0)
	return string(_("Ok"))+eol;

    //translator: these are different kinds of how an rpm package can be broken
    msg = _("Package is not OK for the following reasons:");
    msg += eol;

    if(code&CHK_INCORRECT_VERSION)
    {
	msg += bol;
	msg+=_("Package contains different version than expected");
	msg += eol;
    }
    if(code&CHK_INCORRECT_FILEMD5)
    {
	msg += bol;
	msg+=_("Package file has incorrect MD5 sum");
	msg += eol;
    }
    if(code&CHK_GPGSIG_MISSING)
    {
	msg += bol;
	msg+=_("Package is not signed");
	msg += eol;
    }
    if(code&CHK_MD5SUM_MISSING)
    {
	msg += bol;
	msg+=_("Package has no MD5 sum");
	msg += eol;
    }
    if(code&CHK_INCORRECT_GPGSIG)
    {
	msg += bol;
	msg+=_("Package has incorrect signature");
	msg += eol;
    }
    if(code&CHK_INCORRECT_PKGMD5)
    {
	msg += bol;
	msg+=_("Package archive has incorrect MD5 sum");
	msg += eol;
    }
    if(code&CHK_OTHER_FAILURE)
    {
	msg += bol;
	msg+=_("rpm failed for unkown reason, see log file");
	msg += eol;
    }

    return msg;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::backupPackage
//	METHOD TYPE : bool
//
bool RpmDb::backupPackage( const Pathname & filename )
{
  constRpmHeaderPtr h( RpmHeader::readPackage( filename, RpmHeader::NOSIGNATURE ) );
  if( ! h )
    return false;

  return backupPackage( h->tag_name() );
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmDb::backupPackage
//	METHOD TYPE : bool
//
bool RpmDb::backupPackage(const string& packageName)
{
    Logfile progresslog;
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

    if (PathInfo::assert_dir(_root + _backuppath) != 0)
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
		+ stringutil::form("%s-%d-%d.tar.gz",packageName.c_str(), date, num);

	}
	while ( PathInfo(backupFilename).isExist() && num++ < 1000);

	PathInfo pi(filestobackupfile);
	if(pi.isExist() && !pi.isFile())
	{
	    ERR << filestobackupfile.asString() << " already exists and is no file" << endl;
	    return false;
	}

	std::ofstream fp ( filestobackupfile.asString().c_str(), std::ios::out|std::ios::trunc );

	if(!fp)
	{
	    ERR << "could not open " << filestobackupfile.asString() << endl;
	    return false;
	}

	for (FileList::const_iterator cit = fileList.begin();
	    cit != fileList.end(); ++cit)
	{
	    string name = *cit;
	    if ( name[0] == '/' )
	    {
		// remove slash, file must be relative to -C parameter of tar
		name = name.substr( 1 );
	    }
	    D__ << "saving file "<< name << endl;
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

	string tarmsg;

	// TODO: its probably possible to start tar with -v and watch it adding
	// files to report progress
	for (string output = tar.receiveLine(); output.length() ;output = tar.receiveLine())
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
	    progresslog(/*timestamp*/true) << stringutil::form(_("created backup %s"), backupFilename.asString().c_str()) << endl;
	}

	PathInfo::unlink(filestobackupfile);
    }

    return ret;
}

void RpmDb::setBackupPath(const Pathname& path)
{
    _backuppath = path;
}
