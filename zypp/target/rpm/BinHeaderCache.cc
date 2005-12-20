/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/target/rpm/BinHeaderCache.cc
 *
*/
#include "librpm.h"

#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/Date.h"
#include "zypp/PathInfo.h"
#include "zypp/base/String.h"

#include "zypp/target/rpm/BinHeader.h"
#include "zypp/target/rpm/BinHeaderCache.h"

using namespace std;

namespace zypp {
  namespace target {
    namespace rpm {

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : BinHeaderCache::Cache
      /**
       *
       **/
      class BinHeaderCache::Cache {
      
        friend std::ostream & operator<<( std::ostream & str, const Cache & obj );
      
        Cache & operator=( const Cache & );
        Cache            ( const Cache & );
      
        private:
      
          FD_t  fd;
          pos   _fdpos;    // keep track of filepos for tell()
      
        public:
      
          Cache() : fd( 0 ), _fdpos( ~0 ) {}
          ~Cache() { close(); }
      
        public:
      
          bool open( const Pathname & file_r );
      
          bool isOpen() const { return( fd != 0 ); }
      
          void close();
      
        public:
      
          pos tell() const;
      
          pos seek( const pos pos_r );
      
          unsigned readData( void * buf_r, unsigned count_r );
      
          Header readHeader( bool magicp = true );
      };
      
      ///////////////////////////////////////////////////////////////////
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //	METHOD NAME : BinHeaderCache::Cache::open
      //	METHOD TYPE : bool
      //
      bool BinHeaderCache::Cache::open( const Pathname & file_r )
      {
        close();
      
        switch ( zipType( file_r ) ) {
        case filesystem::ZT_NONE:
          {
            fd = ::Fopen( file_r.asString().c_str(), "r.fdio"  );
            DBG << "PLAIN: open 'r.fdio' " << fd << endl;
          }
          break;
        case filesystem::ZT_GZ:
          {
            fd = ::Fopen( file_r.asString().c_str(), "r.gzdio"  );
            DBG << "GZIP: open 'r.gzdio' " << fd << endl;
          }
          break;
        case filesystem::ZT_BZ2:
          {
            ERR << "BZIP2 is not supported: " << file_r << endl;
      #warning Check BZIP2 support
            break;
            fd = ::Fopen( file_r.asString().c_str(), "r.bzdio"  );
            DBG << "BZIP2: open 'r.bzdio' " << fd << endl;
          }
          break;
        }
      
        if ( fd == 0 || ::Ferror(fd) ) {
          ERR << "Can't open cache for reading: " << file_r << " (" << ::Fstrerror(fd) << ")" << endl;
          close();
          return false;
        }
        _fdpos = 0;
        return true;
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //	METHOD NAME : BinHeaderCache::Cache::close
      //	METHOD TYPE : void
      //
      void BinHeaderCache::Cache::close()
      {
        if ( fd ) {
          ::Fclose( fd );
          fd = 0;
          _fdpos = ~0;
        }
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //	METHOD NAME : BinHeaderCache::Cache::tell
      //	METHOD TYPE : pos
      //
      
      extern "C" {
        typedef struct X_FDSTACK_s {
          FDIO_t	io;
          void *	fp;
          int		fdno;
        } XFDSTACK_t;
      
        struct X_FD_s {
          int		nrefs;
          int		flags;
          int		magic;
      #define	XFDMAGIC	0x04463138
          int		nfps;
          XFDSTACK_t	fps[8];
        };
      }
      
      BinHeaderCache::pos BinHeaderCache::Cache::tell() const
      {
        pos rc = npos;
      
        struct X_FD_s * xfd = (struct X_FD_s*)fd;
      
        if ( !xfd || xfd->magic != XFDMAGIC) {
          INT << "magic(" << XFDMAGIC << ") failed: " << xfd->magic << endl;
          return rc;
        }
      
        return _fdpos;
      #if 0
        if ( xfd->fps[xfd->nfps].io == fpio ) {
          FILE * fp = (FILE *)xfd->fps[xfd->nfps].fp;
          rc = ftell(fp);
        }
      
        if ( rc == npos )
          WAR << "Can't tell:" << ::Ferror(fd) << " (" << ::Fstrerror(fd) << ")" << endl;
        return rc;
      #endif
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //	METHOD NAME : BinHeaderCache::Cache::seek
      //	METHOD TYPE : pos
      //
      BinHeaderCache::pos BinHeaderCache::Cache::seek( const pos pos_r )
      {
        pos rc = npos;
      
        if ( pos_r != npos ) {
          ::Fseek( fd, pos_r, SEEK_SET );
          _fdpos = pos_r;
          if ( tell() == pos_r )
            rc = pos_r;
        } else {
          INT << "Attempt to seek to pos -1" << endl;
        }
      
        if ( rc == npos )
          WAR << "Can't seek to " << pos_r << ":" << ::Ferror(fd) << " (" << ::Fstrerror(fd) << ")" << endl;
        return rc;
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //	METHOD NAME : BinHeaderCache::Cache::readData
      //	METHOD TYPE : unsigned
      //
      unsigned BinHeaderCache::Cache::readData( void * buf_r, unsigned count_r )
      {
        if ( !buf_r ) {
          INT << "Attempt to fill NULL buffer" << endl;
          return 0;
        }
        if ( !count_r ) {
          return 0;
        }
      
        unsigned got = ::Fread( buf_r, sizeof(char), count_r, fd );
        _fdpos += got;
        if ( got != count_r ) {
          if ( got || ::Ferror(fd) ) {
            ERR << "Error reading " << count_r << " byte (" << ::Fstrerror(fd) << ")" << endl;
          } // else EOF?
          return 0;
        }
        return count_r;
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //	METHOD NAME : BinHeaderCache::Cache::readHeader
      //	METHOD TYPE : Header
      //
      
      extern "C" {
      #include <netinet/in.h>
        // from rpm: lib/header.c
        struct XXentryInfo {
          int_32 tag;
          int_32 type;
          int_32 offset;              /* Offset from beginning of data segment,
      				 only defined on disk */
          int_32 count;
        };
      }
      
      Header BinHeaderCache::Cache::readHeader( bool magicp )
      {
        static const int_32 rpm_header_magic = 0x01e8ad8e;
      
        int_32 block[4];
        int_32 il, dl;
        unsigned totalSize = 0;
      
        unsigned count = (magicp ? 4 : 2) * sizeof(int_32);
        if ( readData( block, count ) != count ) {
          ERR << "Error reading header info (" << ::Fstrerror(fd) << ")" << endl;
          return 0;
        }
      
        count = 0;
      
        if ( magicp ) {
          if ( block[count] != rpm_header_magic ) {
            ERR << "Error bad header magic " << str::hexstring( block[count] )
      	<< " (" << str::hexstring( rpm_header_magic ) << ")" << endl;
            return 0;
          }
          count += 2;
        }
      
        il = ntohl( block[count++] );
        dl = ntohl( block[count++] );
      
        totalSize = (2*sizeof(int_32)) + (il * sizeof(struct XXentryInfo)) + dl;
        if (totalSize > (32*1024*1024)) {
          ERR << "Error header ecxeeds 32Mb limit (" << totalSize << ")" << endl;
          return NULL;
        }
      
        char * data = new char[totalSize];
        int_32 * p = (int_32 *)data;
        Header h = 0;
      
        *p++ = htonl(il);
        *p++ = htonl(dl);
        totalSize -= (2*sizeof(int_32));
      
        if ( readData( p, totalSize ) != totalSize ) {
          ERR << "Error reading header data (" << ::Fstrerror(fd) << ")" << endl;
        } else {
          h = ::headerCopyLoad( data );
          if ( !h ) {
            ERR << "Error loading header data" << endl;
          }
        }
      
        delete [] data;
      
        return h;
      }
      
      
      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : BinHeaderCache
      //
      ///////////////////////////////////////////////////////////////////
      
      const unsigned BinHeaderCache::BHC_MAGIC_SZE( 64 );
      
      const BinHeaderCache::pos BinHeaderCache::npos( BinHeaderCache::pos(-1) );
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //	METHOD NAME : BinHeaderCache::BinHeaderCache
      //	METHOD TYPE : Constructor
      //
      BinHeaderCache::BinHeaderCache( const Pathname & cache_r )
          : _c( * new Cache )
          , _cpath( cache_r )
          , _cdate( 0 )
          , _cheaderStart( npos )
      {
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //	METHOD NAME : BinHeaderCache::~BinHeaderCache
      //	METHOD TYPE : Destructor
      //
      BinHeaderCache::~BinHeaderCache()
      {
        delete &_c;
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //	METHOD NAME : BinHeaderCache::_cReadMagic
      //	METHOD TYPE : int
      //
      //      [string\0][string\0][\0padded]
      //
      int BinHeaderCache::_cReadMagic()
      {
        char magic[BHC_MAGIC_SZE+1];
        memset( magic, 0, BHC_MAGIC_SZE+1 );
      
        if ( _c.readData( magic, BHC_MAGIC_SZE ) != BHC_MAGIC_SZE ) {
          ERR << "Error reading magic of cache file " << _cpath  << endl;
          return -1;
        }
      
        _cmagic = magic;
        if ( _cmagic.size() < BHC_MAGIC_SZE ) {
          _cdate = strtoul( magic+_cmagic.size()+1, 0, 10 );
          if ( _cdate ) {
            _cheaderStart = BHC_MAGIC_SZE;
            return 0;
          }
        }
      
        ERR << "No magic in cache file " << _cpath  << endl;
        return -2;
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //	METHOD NAME : BinHeaderCache::open
      //	METHOD TYPE : bool
      //
      bool BinHeaderCache::open()
      {
        if ( _c.isOpen() )
          return true;
      
        if ( !_c.open( _cpath ) ) {
          close();
          return false;
        }
      
        if ( _cReadMagic() != 0 ) {
          close();
          return false;
        }
      
        if ( !magicOk() ) {
          ERR << "Bad magic in cache file " << _cpath  << endl;
          close();
          return false;
        }
      
        return true;
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //	METHOD NAME : BinHeaderCache::isOpen
      //	METHOD TYPE : bool
      //
      bool BinHeaderCache::isOpen() const
      {
        return _c.isOpen();
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //	METHOD NAME : BinHeaderCache::close
      //	METHOD TYPE : void
      //
      void BinHeaderCache::close()
      {
        _cmagic       = "";
        _cdate        = 0;
        _cheaderStart = npos;
        _c.close();
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //	METHOD NAME : BinHeaderCache::tell
      //	METHOD TYPE : BinHeaderCache::pos
      //
      BinHeaderCache::pos BinHeaderCache::tell() const
      {
        return _c.tell();
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //	METHOD NAME : BinHeaderCache::seek
      //	METHOD TYPE : BinHeaderCache::pos
      //
      BinHeaderCache::pos BinHeaderCache::seek( const pos pos_r )
      {
        return _c.seek( pos_r );
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //	METHOD NAME : BinHeaderCache::readData
      //	METHOD TYPE : unsigned
      //
      unsigned BinHeaderCache::readData( void * buf_r, unsigned count_r )
      {
        return _c.readData( buf_r, count_r );
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //	METHOD NAME : BinHeaderCache::readHeader
      //	METHOD TYPE : BinHeaderPtr
      //
      BinHeader::Ptr BinHeaderCache::readHeader( bool magicp )
      {
        Header h = _c.readHeader( magicp );
        if ( !h )
          return 0;
        return new BinHeader( h );
      }
      
      /******************************************************************
      **
      **
      **	FUNCTION NAME : operator<<
      **	FUNCTION TYPE : ostream &
      */
      ostream & operator<<( ostream & str, const BinHeaderCache & obj )
      {
        str << "BinHeaderCache@" << (void *)&obj;
        if ( obj.isOpen() ) {
          str << '(' << obj._cmagic << '|' << Date(obj._cdate) << "|at " << obj._cheaderStart << ')';
        } else {
          str << "(closed)";
        }
        return str;
      }

    } // namespace rpm
  } // namespace target
} // namespace zypp
