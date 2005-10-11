/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/detail/ResolvableImpl.h
 *
*/
#ifndef ZYPP_DETAIL_RESOLVABLEIMPL_H
#define ZYPP_DETAIL_RESOLVABLEIMPL_H

#include <iosfwd>

#include "zypp/ResKind.h"
#include "zypp/ResName.h"
#include "zypp/ResEdition.h"
#include "zypp/ResArch.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ResolvableImpl
    //
    /** */
    class ResolvableImpl
    {
    public:
      /** Default ctor */
      ResolvableImpl()
      {}
      /** ctor */
      ResolvableImpl( const ResKind & kind_r,
                      const ResName & name_r,
                      const ResEdition & edition_r,
                      const ResArch & arch_r )
      : _kind( kind_r )
      , _name( name_r )
      , _edition( edition_r )
      , _arch( arch_r )
      {}
      /** Dtor */
      ~ResolvableImpl()
      {}
    public:
      /**  */
      const ResKind & kind() const
      { return _kind; }
      /**  */
      const ResName & name() const
      { return _name; }
      /**  */
      const ResEdition & edition() const
      { return _edition; }
      /**  */
      const ResArch & arch() const
      { return _arch; }
    private:
      ResKind    _kind;
      ResName    _name;
      ResEdition _edition;
      ResArch    _arch;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_RESOLVABLEIMPL_H
