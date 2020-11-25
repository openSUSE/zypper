/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/target/rpm/librpmDb.cc
 *
*/
#include "librpm.h"

#include <iostream>

#include <zypp/base/Logger.h>
#include <zypp/PathInfo.h>
#include <zypp/target/rpm/librpmDb.h>
#include <zypp/target/rpm/RpmHeader.h>
#include <zypp/target/rpm/RpmException.h>

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "librpmDb"

using std::endl;

namespace zypp
{
namespace target
{
namespace rpm
{
///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : librpmDb::D
/**
 * @short librpmDb internal database handle
 **/
class librpmDb::D
{
  D & operator=( const D & ); // NO ASSIGNMENT!
  D ( const D & );            // NO COPY!
public:

  const Pathname _root;   // root directory for all operations
  const Pathname _dbPath; // directory (below root) that contains the rpmdb
  rpmts _ts;              // transaction handle, includes database
  shared_ptr<RpmException> _error;  // database error

  friend std::ostream & operator<<( std::ostream & str, const D & obj )
  {
    str << "{" << obj._error  << "(" << obj._root << ")" << obj._dbPath << "}";
    return str;
  }

  D( const Pathname & root_r, const Pathname & dbPath_r, bool readonly_r )
      : _root  ( root_r )
      , _dbPath( dbPath_r )
      , _ts    ( 0 )
  {
    _error.reset();
    // set %_dbpath macro
    ::addMacro( NULL, "_dbpath", NULL, _dbPath.asString().c_str(), RMIL_CMDLINE );

    _ts = ::rpmtsCreate();
    ::rpmtsSetRootDir( _ts, _root.c_str() );

    // open database (creates a missing one on the fly)
    int res = ::rpmtsOpenDB( _ts, (readonly_r ? O_RDONLY : O_RDWR ));
    if ( res )
    {
      ERR << "rpmdbOpen error(" << res << "): " << *this << endl;
      _error = shared_ptr<RpmDbOpenException>(new RpmDbOpenException(_root, _dbPath));
      rpmtsFree(_ts);
      ZYPP_THROW(*_error);
      return;
    }

    DBG << "DBACCESS " << *this << endl;
  }

  ~D()
  {
    if ( _ts )
    {
      ::rpmtsFree(_ts);
    }
  }
};

///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : librpmDb (ststic interface)
//
///////////////////////////////////////////////////////////////////

Pathname librpmDb::_defaultRoot { "/" };
Pathname librpmDb::_defaultDbPath;	// set in dbAccess depending on suggestedDbPath below /root
Pathname librpmDb::_rpmDefaultDbPath;	// set by globalInit
librpmDb::constPtr librpmDb::_defaultDb;
bool librpmDb::_dbBlocked = true;

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : librpmDb::globalInit
//	METHOD TYPE : bool
//
bool librpmDb::globalInit()
{
  static bool initialized = false;

  if ( initialized )
    return true;

  int rc = ::rpmReadConfigFiles( NULL, NULL );
  if ( rc )
  {
    ERR << "rpmReadConfigFiles returned " << rc << endl;
    return false;
  }

  initialized = true; // Necessary to be able to use exand().
  _rpmDefaultDbPath = expand( "%{_dbpath}" );

  MIL << "librpm init done: (_target:" << expand( "%{_target}" ) << ") (_dbpath:" << _rpmDefaultDbPath << ")" << endl;
  return initialized;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : librpmDb::expand
//	METHOD TYPE : std::string
//
std::string librpmDb::expand( const std::string & macro_r )
{
  if ( ! globalInit() )
    return macro_r;  // unexpanded

  char * val = ::rpmExpand( macro_r.c_str(), NULL );
  if ( !val )
    return "";

  std::string ret( val );
  free( val );
  return ret;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : librpmDb::newLibrpmDb
//	METHOD TYPE : librpmDb *
//
librpmDb * librpmDb::newLibrpmDb()
{
  // initialize librpm
  if ( ! globalInit() )
  {
    ZYPP_THROW(GlobalRpmInitException());
  }

  // open rpmdb
  librpmDb * ret = 0;
  try
  {
    ret = new librpmDb( _defaultRoot, _defaultDbPath, /*readonly*/true );
  }
  catch (const RpmException & excpt_r)
  {
    ZYPP_CAUGHT(excpt_r);
    delete ret;
    ret = 0;
    ZYPP_RETHROW(excpt_r);
  }
  return ret;
}


Pathname librpmDb::suggestedDbPath( const Pathname & root_r )
{
  if ( ! root_r.absolute() )
    ZYPP_THROW(RpmInvalidRootException( root_r, "" ));

  // initialize librpm (for _rpmDefaultDbPath)
  if ( ! globalInit() )
    ZYPP_THROW(GlobalRpmInitException());

  if ( PathInfo( root_r ).isDir() ) {
    // If a known dbpath exsists, we continue to use it
    for ( auto p : { "/var/lib/rpm", "/usr/lib/sysimage/rpm" } ) {
      if ( PathInfo( root_r/p, PathInfo::LSTAT/*!no symlink*/ ).isDir() ) {
	MIL << "Suggest existing database at " << stringPath( root_r, p ) << endl;
	return p;
      }
    }
  }

  MIL << "Suggest rpm _dbpath " << stringPath( root_r, _rpmDefaultDbPath ) << endl;
  return _rpmDefaultDbPath;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : librpmDb::dbAccess
//	METHOD TYPE : PMError
//
void librpmDb::dbAccess( const Pathname & root_r )
{
  if ( _defaultDb )
  {
    // already accessing a database: switching is not allowed.
    if ( _defaultRoot == root_r )
      return;
    else
      ZYPP_THROW(RpmDbAlreadyOpenException(_defaultRoot, _defaultDbPath, root_r, _defaultDbPath));
  }

  // got no database: we could switch to a new one (even if blocked!)
  _defaultDbPath = suggestedDbPath( root_r );	// also asserts root_r is absolute
  _defaultRoot = root_r;

  MIL << "Set new database location: " << stringPath( _defaultRoot, _defaultDbPath ) << endl;
  return dbAccess();
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : librpmDb::dbAccess
//	METHOD TYPE : PMError
//
void librpmDb::dbAccess()
{
  if ( _dbBlocked )
  {
    ZYPP_THROW(RpmAccessBlockedException(_defaultRoot, _defaultDbPath));
  }

  if ( !_defaultDb )
  {
    // get access
    _defaultDb = newLibrpmDb();
  }
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : librpmDb::dbAccess
//	METHOD TYPE : PMError
//
void librpmDb::dbAccess( librpmDb::constPtr & ptr_r )
{
  ptr_r = nullptr;
  dbAccess();
  ptr_r = _defaultDb;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : librpmDb::dbRelease
//	METHOD TYPE : unsigned
//
unsigned librpmDb::dbRelease( bool force_r )
{
  if ( !_defaultDb )
  {
    return 0;
  }

  unsigned outstanding = _defaultDb->refCount() - 1; // refCount can't be 0

  switch ( outstanding )
  {
  default:
    if ( !force_r )
    {
      DBG << "dbRelease: keep access, outstanding " << outstanding << endl;
      break;
    }
    // else fall through:
  case 0:
    DBG << "dbRelease: release" << (force_r && outstanding ? "(forced)" : "")
    << ", outstanding " << outstanding << endl;

    _defaultDb->_d._error = shared_ptr<RpmAccessBlockedException>(new RpmAccessBlockedException(_defaultDb->_d._root, _defaultDb->_d._dbPath));
    // tag handle invalid
    _defaultDb = 0;
    break;
  }

  return outstanding;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : librpmDb::blockAccess
//	METHOD TYPE : unsigned
//
unsigned librpmDb::blockAccess()
{
  MIL << "Block access" << endl;
  _dbBlocked = true;
  return dbRelease( /*force*/true );
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : librpmDb::unblockAccess
//	METHOD TYPE : void
//
void librpmDb::unblockAccess()
{
  MIL << "Unblock access" << endl;
  _dbBlocked = false;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : librpmDb::dumpState
//	METHOD TYPE : ostream &
//
std::ostream & librpmDb::dumpState( std::ostream & str )
{
  if ( !_defaultDb )
  {
    return str << "[librpmDb " << (_dbBlocked?"BLOCKED":"CLOSED") << " " << stringPath( _defaultRoot, _defaultDbPath ) << "]";
  }
  return str << "[" << _defaultDb << "]";
}

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : librpmDb (internal database handle interface (nonstatic))
//
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : librpmDb::librpmDb
//	METHOD TYPE : Constructor
//
//	DESCRIPTION :
//
librpmDb::librpmDb( const Pathname & root_r, const Pathname & dbPath_r, bool readonly_r )
    : _d( * new D( root_r, dbPath_r, readonly_r ) )
{}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : librpmDb::~librpmDb
//	METHOD TYPE : Destructor
//
//	DESCRIPTION :
//
librpmDb::~librpmDb()
{
  delete &_d;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : librpmDb::unref_to
//	METHOD TYPE : void
//
void librpmDb::unref_to( unsigned refCount_r ) const
{
  if ( refCount_r == 1 )
  {
    dbRelease();
  }
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : librpmDb::root
//	METHOD TYPE : const Pathname &
//
const Pathname & librpmDb::root() const
{
  return _d._root;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : librpmDb::dbPath
//	METHOD TYPE : const Pathname &
//
const Pathname & librpmDb::dbPath() const
{
  return _d._dbPath;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : librpmDb::error
//	METHOD TYPE : PMError
//
shared_ptr<RpmException> librpmDb::error() const
{
  return _d._error;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : librpmDb::empty
//	METHOD TYPE : bool
//
bool librpmDb::empty() const
{
  return( valid() && ! *db_const_iterator( this ) );
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : librpmDb::size
//	METHOD TYPE : unsigned
//
unsigned librpmDb::size() const
{
  unsigned count = 0;
  if ( valid() )
  {
    db_const_iterator it( this );
    for ( db_const_iterator it( this ); *it; ++it )
      ++count;
  }
  return count;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : librpmDb::dont_call_it
//	METHOD TYPE : void *
//
void * librpmDb::dont_call_it() const
{
  return rpmtsGetRdb(_d._ts);
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : librpmDb::dumpOn
//	METHOD TYPE : ostream &
//
//	DESCRIPTION :
//
std::ostream & librpmDb::dumpOn( std::ostream & str ) const
{
  ReferenceCounted::dumpOn( str ) << _d;
  return str;
}

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : librpmDb::db_const_iterator::D
/**
 *
 **/
class librpmDb::db_const_iterator::D
{
  D & operator=( const D & ); // NO ASSIGNMENT!
  D ( const D & );            // NO COPY!
public:

  librpmDb::constPtr     _dbptr;
  shared_ptr<RpmException> _dberr;

  RpmHeader::constPtr _hptr;
  rpmdbMatchIterator   _mi;

  D( librpmDb::constPtr dbptr_r )
      : _dbptr( dbptr_r )
      , _mi( 0 )
  {
    if ( !_dbptr )
    {
      try
      {
        librpmDb::dbAccess( _dbptr );
      }
      catch (const RpmException & excpt_r)
      {
        ZYPP_CAUGHT(excpt_r);
      }
      if ( !_dbptr )
      {
        WAR << "No database access: " << _dberr << endl;
      }
    }
    else
    {
      destroy(); // Checks whether _dbptr still valid
    }
  }

  ~D()
  {
    if ( _mi )
    {
      ::rpmdbFreeIterator( _mi );
    }
  }

  /**
   * Let iterator access a dbindex file. Call @ref advance to access the
   * 1st element (if present).
   **/
  bool create( int rpmtag, const void * keyp = NULL, size_t keylen = 0 )
  {
    destroy();
    if ( ! _dbptr )
      return false;
    _mi = ::rpmtsInitIterator( _dbptr->_d._ts, rpmTag(rpmtag), keyp, keylen );
    return _mi;
  }

  /**
   * Destroy iterator. Invalidates _dbptr, if database was blocked meanwile.
   * Always returns false.
   **/
  bool destroy()
  {
    if ( _mi )
    {
      _mi = ::rpmdbFreeIterator( _mi );
      _hptr = 0;
    }
    if ( _dbptr && _dbptr->error() )
    {
      _dberr = _dbptr->error();
      WAR << "Lost database access: " << _dberr << endl;
      _dbptr = 0;
    }
    return false;
  }

  /**
   * Advance to the first/next header in iterator. Destroys iterator if
   * no more headers available.
   **/
  bool advance()
  {
    if ( !_mi )
      return false;
    Header h = ::rpmdbNextIterator( _mi );
    if ( ! h )
    {
      destroy();
      return false;
    }
    _hptr = new RpmHeader( h );
    return true;
  }

  /**
   * Access a dbindex file and advance to the 1st header.
   **/
  bool init( int rpmtag, const void * keyp = NULL, size_t keylen = 0 )
  {
    if ( ! create( rpmtag, keyp, keylen ) )
      return false;
    return advance();
  }

  /**
   * Create an itertator that contains the database entry located at
   * off_r, and advance to the 1st header.
   **/
  bool set( int off_r )
  {
    if ( ! create( RPMDBI_PACKAGES ) )
      return false;
#ifdef RPMFILEITERMAX	// since rpm.4.12
    ::rpmdbAppendIterator( _mi, (const unsigned *)&off_r, 1 );
#else
    ::rpmdbAppendIterator( _mi, &off_r, 1 );
#endif
    return advance();
  }

  unsigned offset()
  {
    return( _mi ? ::rpmdbGetIteratorOffset( _mi ) : 0 );
  }

  int size()
  {
    if ( !_mi )
      return 0;
    int ret = ::rpmdbGetIteratorCount( _mi );
    return( ret ? ret : -1 ); // -1: sequential access
  }
};

///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : librpmDb::Ptr::db_const_iterator
//
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : librpmDb::db_const_iterator::db_iterator
//	METHOD TYPE : Constructor
//
librpmDb::db_const_iterator::db_const_iterator( librpmDb::constPtr dbptr_r )
    : _d( * new D( dbptr_r ) )
{
  findAll();
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : librpmDb::db_const_iterator::~db_const_iterator
//	METHOD TYPE : Destructor
//
librpmDb::db_const_iterator::~db_const_iterator()
{
  delete &_d;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : librpmDb::db_const_iterator::operator++
//	METHOD TYPE : void
//
void librpmDb::db_const_iterator::operator++()
{
  _d.advance();
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : librpmDb::db_const_iterator::dbHdrNum
//	METHOD TYPE : unsigned
//
unsigned librpmDb::db_const_iterator::dbHdrNum() const
{
  return _d.offset();
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : librpmDb::db_const_iterator::operator*
//	METHOD TYPE : const RpmHeader::constPtr &
//
const RpmHeader::constPtr & librpmDb::db_const_iterator::operator*() const
{
  return _d._hptr;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : librpmDb::db_const_iterator::dbError
//	METHOD TYPE : PMError
//
shared_ptr<RpmException> librpmDb::db_const_iterator::dbError() const
{
  if ( _d._dbptr )
    return _d._dbptr->error();

  return _d._dberr;
}

/******************************************************************
**
**
**	FUNCTION NAME : operator<<
**	FUNCTION TYPE : ostream &
*/
std::ostream & operator<<( std::ostream & str, const librpmDb::db_const_iterator & obj )
{
  str << "db_const_iterator(" << obj._d._dbptr
  << " Size:" << obj._d.size()
  << " HdrNum:" << obj._d.offset()
  << ")";
  return str;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : librpmDb::db_const_iterator::findAll
//	METHOD TYPE : bool
//
bool librpmDb::db_const_iterator::findAll()
{
  return _d.init( RPMDBI_PACKAGES );
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : librpmDb::db_const_iterator::findByFile
//	METHOD TYPE : bool
//
bool librpmDb::db_const_iterator::findByFile( const std::string & file_r )
{
  return _d.init( RPMTAG_BASENAMES, file_r.c_str() );
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : librpmDb::db_const_iterator::findByProvides
//	METHOD TYPE : bool
//
bool librpmDb::db_const_iterator::findByProvides( const std::string & tag_r )
{
  return _d.init( RPMTAG_PROVIDENAME, tag_r.c_str() );
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : librpmDb::db_const_iterator::findByRequiredBy
//	METHOD TYPE : bool
//
bool librpmDb::db_const_iterator::findByRequiredBy( const std::string & tag_r )
{
  return _d.init( RPMTAG_REQUIRENAME, tag_r.c_str() );
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : librpmDb::db_const_iterator::findByConflicts
//	METHOD TYPE : bool
//
bool librpmDb::db_const_iterator::findByConflicts( const std::string & tag_r )
{
  return _d.init( RPMTAG_CONFLICTNAME, tag_r.c_str() );
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : librpmDb::findByName
//	METHOD TYPE : bool
//
bool librpmDb::db_const_iterator::findByName( const std::string & name_r )
{
  return _d.init( RPMTAG_NAME, name_r.c_str() );
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : librpmDb::db_const_iterator::findPackage
//	METHOD TYPE : bool
//
bool librpmDb::db_const_iterator::findPackage( const std::string & name_r )
{
  if ( ! _d.init( RPMTAG_NAME, name_r.c_str() ) )
    return false;

  if ( _d.size() == 1 )
    return true;

  // check installtime on multiple entries
  int match    = 0;
  time_t itime = 0;
  for ( ; operator*(); operator++() )
  {
    if ( operator*()->tag_installtime() > itime )
    {
      match = _d.offset();
      itime = operator*()->tag_installtime();
    }
  }

  return _d.set( match );
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : librpmDb::db_const_iterator::findPackage
//	METHOD TYPE : bool
//
bool librpmDb::db_const_iterator::findPackage( const std::string & name_r, const Edition & ed_r )
{
  if ( ! _d.init( RPMTAG_NAME, name_r.c_str() ) )
    return false;

  for ( ; operator*(); operator++() )
  {
    if ( ed_r == operator*()->tag_edition() )
    {
      int match = _d.offset();
      return _d.set( match );
    }
  }

  return _d.destroy();
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : librpmDb::db_const_iterator::findPackage
//	METHOD TYPE : bool
//
bool librpmDb::db_const_iterator::findPackage( const Package::constPtr & which_r )
{
  if ( ! which_r )
    return _d.destroy();

  return findPackage( which_r->name(), which_r->edition() );
}

} // namespace rpm
} // namespace target
} // namespace zypp
