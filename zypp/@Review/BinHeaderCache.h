/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/target/rpm/BinHeaderCache.h
 *
*/
#ifndef ZYPP_TARGET_BINHEADERCACHE_H
#define ZYPP_TARGET_BINHEADERCACHE_H

#include <iosfwd>

#include "zypp/Pathname.h"

#include "zypp/target/rpm/BinHeader.h"

namespace zypp {
  namespace target {
    namespace rpm {

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : BinHeaderCache
      /**
       *
       **/
      class BinHeaderCache {
      
        friend std::ostream & operator<<( std::ostream & str, const BinHeaderCache & obj );
      
        BinHeaderCache & operator=( const BinHeaderCache & );
        BinHeaderCache            ( const BinHeaderCache & );
      
        public:
      
          typedef unsigned pos;
      
          static const pos npos;
      
        private:
      
          static const unsigned BHC_MAGIC_SZE;
      
          class Cache;
      
          Cache & _c;
      
        private:
      
          int _cReadMagic();
      
        protected:
      
          Pathname _cpath;
      
        protected:
      
          std::string _cmagic;
      
          time_t _cdate;
      
          pos _cheaderStart;
      
        protected:
      
          virtual bool magicOk() { return _cmagic.empty(); }
      
        public:
      
          BinHeaderCache( const Pathname & cache_r );
      
          virtual ~BinHeaderCache();
      
        public:
      
          bool open();
      
          void close();
      
          bool isOpen() const;
      
          const Pathname & cpath() const { return  _cpath; }
      
          const std::string & cmagic() const { return _cmagic; }
      
          time_t cdate() const { return _cdate; }
      
          pos tell() const;
      
          pos seek( const pos pos_r );
      
          unsigned readData( void * buf_r, unsigned count_r );
      
          BinHeader::Ptr readHeader( bool magicp = true );
      };
      
      ///////////////////////////////////////////////////////////////////

    } // namespace rpm
  } // namespace target
} // namespace zypp

#endif // ZYPP_TARGET_BINHEADERCACHE_H
