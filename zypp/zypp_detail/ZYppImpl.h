/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/zypp_detail/ZYppImpl.h
 *
*/
#ifndef ZYPP_ZYPP_DETAIL_ZYPPIMPL_H
#define ZYPP_ZYPP_DETAIL_ZYPPIMPL_H

#include <iosfwd>

#include "zypp/ResPoolManager.h"
#include "zypp/SourceManager.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace zypp_detail
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ZYppImpl
    //
    /** */
    class ZYppImpl
    {
      friend std::ostream & operator<<( std::ostream & str, const ZYppImpl & obj );

    public:
      /** Default ctor */
      ZYppImpl();
      /** Dtor */
      ~ZYppImpl();

    public:
      /** */
      ResPool pool() const
      { return _pool.accessor(); }

      void addResolvables (const ResStore& store);

      void removeResolvables (const ResStore& store);
            
    private:
      /** */
      ResPoolManager _pool;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates ZYppImpl Stream output */
    std::ostream & operator<<( std::ostream & str, const ZYppImpl & obj );

    /////////////////////////////////////////////////////////////////
  } // namespace zypp_detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_ZYPP_DETAIL_ZYPPIMPL_H
