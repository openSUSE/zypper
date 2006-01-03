/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/target/rpm/RpmHeaderCache.h
 *
*/
#ifndef ZYPP_TARGET_RPM_RPMHEADERCACHE_H
#define ZYPP_TARGET_RPM_RPMHEADERCACHE_H

#include <iosfwd>

#include "zypp/Pathname.h"
#include "zypp/target/rpm/BinHeaderCache.h"
#include "zypp/target/rpm/RpmHeader.h"

namespace zypp {
  namespace target {
    namespace rpm {

      ///////////////////////////////////////////////////////////////////
      //
      //        CLASS NAME : RpmHeaderCache
      /**
       *
       **/
      class RpmHeaderCache : public BinHeaderCache {
      
        friend std::ostream & operator<<( std::ostream & str, const RpmHeaderCache & obj );
      
        RpmHeaderCache & operator=( const RpmHeaderCache & );
        RpmHeaderCache            ( const RpmHeaderCache & );
      
        private:
#warning Add this function if it is needed
#if 0      
          static const PkgNameEd & def_magic();
      
        protected:
      
          virtual bool magicOk();
#endif
      
        public:
      
          RpmHeaderCache( const Pathname & cache_r );
          virtual ~RpmHeaderCache();
      
          RpmHeader::constPtr getFirst( Pathname & citem_r, int & isSource_r, pos & at_r );
          RpmHeader::constPtr getNext( Pathname & citem_r, int & isSource_r, pos & at_r );
      
          RpmHeader::constPtr getAt( pos at_r );
      
        public:
      
          struct buildOpts {
            bool recurse;
            buildOpts()
              : recurse( false )
            {}
          };
      
          static int buildHeaderCache( const Pathname & cache_r, const Pathname & pkgroot_r,
              			 const buildOpts & options_r = buildOpts() );
      };
      
      ///////////////////////////////////////////////////////////////////
      
    } // namespace rpm
  } // namespace target
} // namespace zypp

#endif // ZYPP_TARGET_RPM_RPMHEADERCACHE_H
