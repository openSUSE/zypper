/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/cache/CacheFSCK.h
 *
*/
#ifndef ZYPP2_CACHE_CACHEFSCK_H
#define ZYPP2_CACHE_CACHEFSCK_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"
#include "zypp/Pathname.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace cache
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : CacheFSCK
    //
    /** 
     * Check for consistency of the cache
     **/
    class CacheFSCK
    {
      friend std::ostream & operator<<( std::ostream & str, const CacheFSCK & obj );

    public:
      /** Implementation  */
      class Impl;

    public:
      /**
       * Default ctor
       *
       * \param dbdir Cache directory
       */
      CacheFSCK( const Pathname &dbdir );
      
      void start();
      /** Dtor */
      ~CacheFSCK();

    public:

    private:
      /** Pointer to implementation */
      RWCOW_pointer<Impl> _pimpl;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates CacheFSCK Stream output */
    std::ostream & operator<<( std::ostream & str, const CacheFSCK & obj );

    /////////////////////////////////////////////////////////////////
  } // namespace cache
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP2_CACHE_CACHEFSCK_H
