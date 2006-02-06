/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ui/ResPoolProxy.h
 *
*/
#ifndef ZYPP_UI_RESPOOLPROXY_H
#define ZYPP_UI_RESPOOLPROXY_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"

#include "zypp/ResPool.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace ui
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ResPoolProxy
    //
    /**

     * \todo Make it a _Ref.
    */
    class ResPoolProxy
    {
      friend std::ostream & operator<<( std::ostream & str, const ResPoolProxy & obj );

    public:
      /** Implementation  */
      class Impl;

    public:
      /** Default ctor: no pool */
      ResPoolProxy();
      /** Ctor */
      ResPoolProxy( ResPool_Ref pool_r );
      /** Dtor */
      ~ResPoolProxy();

    public:




    private:
      /** Pointer to implementation */
      RW_pointer<Impl> _pimpl;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates ResPoolProxy Stream output */
    std::ostream & operator<<( std::ostream & str, const ResPoolProxy & obj );

    /////////////////////////////////////////////////////////////////
  } // namespace ui
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_UI_RESPOOLPROXY_H
