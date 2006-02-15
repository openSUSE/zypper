/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/detail/ImplConnect.h
 *
*/
#ifndef ZYPP_DETAIL_IMPLCONNECT_H
#define ZYPP_DETAIL_IMPLCONNECT_H

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ImplConnect
    //
    /** */
    struct ImplConnect
    {
      template<class _Res>
        static typename _Res::Impl & resimpl( _Res & obj )
        { return dynamic_cast<typename _Res::Impl &>( static_cast<ResObject &>(obj) ); }

      template<class _Res>
        static const typename _Res::Impl & resimpl( const _Res & obj )
        { return dynamic_cast<const typename _Res::Impl &>( static_cast<const ResObject &>(obj) ); }

      static ResObject::Impl & resimpl( ResObject & obj )
      { return obj.pimpl(); }

      static const ResObject::Impl & resimpl( const ResObject & obj )
      { return obj.pimpl(); }
     };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_IMPLCONNECT_H
