/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/detail/ResObjectFactory.h
 *
*/
#ifndef ZYPP_DETAIL_RESOBJECTFACTORY_H
#define ZYPP_DETAIL_RESOBJECTFACTORY_H

#include "zypp/base/PtrTypes.h"
#include "zypp/NVRAD.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class Edition;
  class Arch;
  struct NVRAD;

  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace _resobjectfactory_detail
    { /////////////////////////////////////////////////////////////////

      /** Glues a Resolvable to it's implementation. */
      template<class _Res>
        class ResImplConnect : public _Res
        {
        public:
          typedef ResImplConnect      Self;
          typedef typename _Res::Impl Impl;
          typedef shared_ptr<Impl>    Impl_Ptr;
          // Ptr types not needed
          // typedef intrusive_ptr<Self>       Ptr;
          // typedef intrusive_ptr<const Self> constPtr;
        public:
          /** Ctor */
          ResImplConnect( const NVRAD & nvrad_r,
                          Impl_Ptr impl_r )
          : _Res( nvrad_r )
          , _impl( impl_r )
          { _impl->_backRef = this; }

          virtual ~ResImplConnect()
          { _impl->_backRef = 0; }

        private:
          Impl_Ptr _impl;
          virtual Impl &       pimpl()       { return *_impl; }
          virtual const Impl & pimpl() const { return *_impl; }
        };

    /////////////////////////////////////////////////////////////////
    } // namespace _resobjectfactory
    ///////////////////////////////////////////////////////////////////

    template<class _Impl>
      typename _Impl::ResType::Ptr
      makeResolvableAndImpl( const NVRAD & nvrad_r,
                             shared_ptr<_Impl> & impl_r )
      {
        impl_r.reset( new _Impl );
        return new
               _resobjectfactory_detail::ResImplConnect<typename _Impl::ResType>
               ( nvrad_r, impl_r );
      }

    template<class _Impl>
      typename _Impl::ResType::Ptr
      makeResolvableAndImpl( const std::string & name_r,
                             const Edition & edition_r,
                             const Arch & arch_r,
                             shared_ptr<_Impl> & impl_r )
      {
        return makeResolvableAndImpl( NVRAD( name_r, edition_r, arch_r ), impl_r );
      }

    template<class _Impl>
      typename _Impl::ResType::Ptr
      makeResolvableFromImpl( const NVRAD & nvrad_r,
                              shared_ptr<_Impl> impl_r )
      {
        if ( ! impl_r )
          throw ( "makeResolvableFromImpl: NULL Impl " );
        if ( impl_r->hasBackRef() )
          throw ( "makeResolvableFromImpl: Impl already managed" );
        return new
               _resobjectfactory_detail::ResImplConnect<typename _Impl::ResType>
               ( nvrad_r, impl_r );
      }

    template<class _Impl>
      typename _Impl::ResType::Ptr
      makeResolvableFromImpl( const std::string & name_r,
                              const Edition & edition_r,
                              const Arch & arch_r,
                              shared_ptr<_Impl> impl_r )
      {
        return makeResolvableFromImpl( NVRAD( name_r, edition_r, arch_r ), impl_r );
      }

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_RESOBJECTFACTORY_H
