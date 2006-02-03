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
    /** */
    class ResPoolProxy
    {
      friend std::ostream & operator<<( std::ostream & str, const ResPoolProxy & obj );

    public:
      /** Implementation  */
      class Impl;

    public:
      /** Default ctor */
      ResPoolProxy();
      /** Dtor */
      ~ResPoolProxy();

    public:

    private:
      /** Pointer to implementation */
      RWCOW_pointer<Impl> _pimpl;
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
