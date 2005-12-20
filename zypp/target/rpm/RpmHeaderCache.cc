/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/target/rpm/RpmHeaderCache.cc
 *
*/

#include <iostream>

#include "zypp/base/Logger.h"

#include "zypp/target/rpm/RpmHeaderCache.h"
#include "zypp/target/rpm/RpmHeader.h"
#include "librpm.h"
#include "zypp/PathInfo.h"

using namespace std;

namespace zypp {
  namespace target {
    namespace rpm {

///////////////////////////////////////////////////////////////////

#warning add this function if needed
#if 0
const PkgNameEd & RpmHeaderCache::def_magic()
{
  static PkgNameEd _def_magic( PkgName("YaST-PHC"), PkgEdition("1.0-0") );
  return _def_magic;
}
#endif

///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmHeaderCache::RpmHeaderCache
//	METHOD TYPE : Constructor
//
RpmHeaderCache::RpmHeaderCache( const Pathname & cache_r )
    : BinHeaderCache( cache_r )
{
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmHeaderCache::~RpmHeaderCache
//	METHOD TYPE : Destructor
//
RpmHeaderCache::~RpmHeaderCache()
{
}

#warning add this function if needed
#if 0
///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmHeaderCache::magicOk
//	METHOD TYPE : bool
//
bool RpmHeaderCache::magicOk()
{
  PkgNameEd magic( PkgNameEd::fromString( _cmagic ) );
  if ( magic != def_magic() ) {
    ERR << "Found magic " << magic << ", expected " << def_magic() << endl;
    return false;
  }
  DBG << "Found magic " << magic << endl;
  return true;
}
#endif

///////////////////////////////////////////////////////////////////
#define RETURN_IF_CLOSED(R) if ( !isOpen() ) { ERR << "Cache not open: " << _cpath << endl; return R; }
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmHeaderCache::getFirst
//	METHOD TYPE : constRpmHeaderPtr
//
RpmHeader::constPtr RpmHeaderCache::getFirst( Pathname & citem_r, int & isSource_r, pos & at_r )
{
  RETURN_IF_CLOSED( (RpmHeader*)0 );

  if ( seek( _cheaderStart ) == npos ) {
    ERR << "Can't seek to first header at " << _cheaderStart << endl;
    return (RpmHeader*)0;
  }

  return getNext( citem_r, isSource_r, at_r );
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmHeaderCache::getNext
//	METHOD TYPE : constRpmHeaderPtr
//
RpmHeader::constPtr RpmHeaderCache::getNext( Pathname & citem_r, int & isSource_r, pos & at_r )
{
  RETURN_IF_CLOSED( RpmHeader::constPtr() );

  static const unsigned sigsize = 8;

  char sig[sigsize+1];
  sig[sigsize] = '\0';

  unsigned count = readData( sig, sigsize );
  if ( count != sigsize ) {
    if ( count ) {
      ERR << "Error reading entry." << endl;
    } // else EOF?
    return (RpmHeader*)0;
  }

  if ( sig[0] != '@' || sig[sigsize-1] != '@' ) {
    ERR << "Invalid entry." << endl;
    return (RpmHeader*)0;
  }

  sig[sigsize-1] = '\0';
  count = atoi( &sig[1] );

  char citem[count+1];
  citem[count] = '\0';

  if ( readData( citem, count ) != count ) {
    ERR << "Error reading entry data." << endl;
    return (RpmHeader*)0;
  }

  isSource_r = ( citem[0] == 's' );
  citem_r    = &citem[1];
  at_r       = tell();

  return getAt( at_r );
}


///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmHeaderCache::getAt
//	METHOD TYPE : constRpmHeaderPtr
//
RpmHeader::constPtr RpmHeaderCache::getAt( pos at_r )
{
  RETURN_IF_CLOSED( RpmHeader::constPtr() );

  if ( seek( at_r ) == npos ) {
    ERR << "Can't seek to header at " << at_r << endl;
    return (RpmHeader*)0;
  }

  BinHeader::Ptr bp = readHeader();
  if ( !bp ) {
    ERR << "Can't read header at " << at_r << endl;
    return (RpmHeader*)0;
  }

  return new RpmHeader( bp );
}

/******************************************************************
**
**
**	FUNCTION NAME : operator<<
**	FUNCTION TYPE : ostream &
*/
ostream & operator<<( ostream & str, const RpmHeaderCache & obj )
{
  return str << "RpmHeaderCache@" << static_cast<const BinHeaderCache &>(obj);
}

///////////////////////////////////////////////////////////////////
//
#warning buildHeaderCache needs cleanup
//
///////////////////////////////////////////////////////////////////

static const unsigned PHC_MAGIC_SZE = 64; // dont change!
static const string   PHC_MAGIC( "YaST-PHC-1.0-0" );

/******************************************************************
**
**
**	FUNCTION NAME : phcAddMagic
**	FUNCTION TYPE : void
*/
void phcAddMagic( FD_t fd )
{
  char magic[PHC_MAGIC_SZE];
  memset( magic, 0, PHC_MAGIC_SZE );
  strcpy( magic, PHC_MAGIC.c_str() );
  strcpy( magic+PHC_MAGIC.size()+1, str::numstring( Date::now() ).c_str() );

  ::Fwrite( magic, sizeof(char), PHC_MAGIC_SZE, fd );
}

/******************************************************************
**
**
**	FUNCTION NAME : phcAddHeader
**	FUNCTION TYPE : unsigned
*/
unsigned phcAddHeader( FD_t fd, Header h, const Pathname & citem_r, int isSource )
{
  string entry = str::form( "%c%s", (isSource?'s':'b'), citem_r.asString().c_str() );
  entry = str::form( "@%6zu@%s", entry.size(), entry.c_str() );

  ::Fwrite( entry.c_str(), sizeof(char), entry.size(), fd );
  ::headerWrite( fd, h, HEADER_MAGIC_YES );

  return 1;
}

/******************************************************************
**
**
**	FUNCTION NAME : phcAddFile
**	FUNCTION TYPE : unsigned
*/
unsigned phcAddFile( FD_t fd, const PathInfo & cpath_r, const Pathname & citem_r )
{
  FD_t pkg = ::Fopen( cpath_r.asString().c_str(), "r.ufdio" );
  if ( pkg == 0 || ::Ferror(pkg) ) {
    ERR << "Can't open file for reading: " << cpath_r << " (" << ::Fstrerror(pkg) << ")" << endl;
    if ( pkg )
      ::Fclose( pkg );
    return 0;
  }

  rpmts ts = rpmtsCreate();
  Header h = 0;
  int res = ::rpmReadPackageFile( ts, pkg, cpath_r.path().asString().c_str(), &h );
  ts = rpmtsFree(ts);
  ::Fclose( pkg );

  if ( ! h ) {
    WAR << "Error reading header from " << cpath_r << " error(" << res << ")" << endl;
    return 0;
  }

  RpmHeader::constPtr dummy( new RpmHeader( h ) ); // to handle header free
  headerFree( h ); // clear reference set in ReadPackageFile
  MIL << dummy << " for " << citem_r << endl;

  return phcAddHeader( fd, h, citem_r, dummy->isSrc() );
}

/******************************************************************
**
**
**	FUNCTION NAME : phcScanDir
**	FUNCTION TYPE : unsigned
*/
unsigned phcScanDir( FD_t fd, const PathInfo & cpath_r, const Pathname & prfx_r,
		     const RpmHeaderCache::buildOpts & options_r )
{
  DBG << "SCAN " << cpath_r << " (" << prfx_r << ")" << endl;

  list<string> retlist;
  int res = filesystem::readdir( retlist, cpath_r.path(), false );
  if ( res ) {
    ERR << "Error reading content of " << cpath_r << " (readdir " << res << ")" << endl;
    return 0;
  }

  unsigned count = 0;
  list<string> downlist;

  for ( list<string>::const_iterator it = retlist.begin(); it != retlist.end(); ++it ) {
    PathInfo cpath( cpath_r.path() + *it, PathInfo::LSTAT );
    if ( cpath.isFile() ) {
      count += phcAddFile( fd, cpath, prfx_r + *it );
    } else if ( options_r.recurse && cpath.isDir() ) {
      downlist.push_back( *it );
    }
  }
  retlist.clear();

  for ( list<string>::const_iterator it = downlist.begin(); it != downlist.end(); ++it ) {
    count += phcScanDir( fd, PathInfo(cpath_r.path() + *it), prfx_r + *it, options_r );
  }

  return count;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RpmHeaderCache::buildHeaderCache
//	METHOD TYPE : int
//
int RpmHeaderCache::buildHeaderCache( const Pathname & cache_r,
					 const Pathname & pkgroot_r,
					 const buildOpts & options_r )
{
  ///////////////////////////////////////////////////////////////////
  // Check pkgroot
  ///////////////////////////////////////////////////////////////////

  PathInfo pkgroot( pkgroot_r );
  if ( !pkgroot.isDir() ) {
    ERR << "Not a directory: Pkgroot " << pkgroot << endl;
    return -1;
  }

  ///////////////////////////////////////////////////////////////////
  // Prepare cache file
  ///////////////////////////////////////////////////////////////////
  FD_t fd = ::Fopen( cache_r.asString().c_str(), "w"  );
  if ( fd == 0 || ::Ferror(fd) ) {
    ERR << "Can't open cache for writing: " << cache_r << " (" << ::Fstrerror(fd) << ")" << endl;
    if ( fd )
      ::Fclose( fd );
    return -2;
  }

  phcAddMagic( fd );

  ///////////////////////////////////////////////////////////////////
  // Scan pkgroot_r
  ///////////////////////////////////////////////////////////////////
  MIL << "Start scan below " << pkgroot_r
    << " (recurse=" << (options_r.recurse?"yes":"no")
    << ")" << endl;
  unsigned count = phcScanDir( fd, pkgroot, "/", options_r );

  if ( ::Ferror(fd) ) {
    ERR << "Error writing cache: " << cache_r << " (" << ::Fstrerror(fd) << ")" << endl;
    ::Fclose( fd );
    return -3;
  }

  MIL << "Found " << count << " package(s) below " << pkgroot_r << endl;
  ::Fclose( fd );
  return count;
}

    } // namespace rpm
  } // namespace target
} // namespace zypp
