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

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class Edition;
  class Arch;

  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace _resobjectfactory_detail
    { /////////////////////////////////////////////////////////////////

      template<class _Res>
        class ResImplConnect : public _Res
        {
        public:
          typedef ResImplConnect                  Self;
          typedef typename _Res::Impl             Impl;
          typedef base::shared_ptr<Impl>          Impl_Ptr;
          // Ptr types not needed
          // typedef base::intrusive_ptr<Self>       Ptr;
          // typedef base::intrusive_ptr<const Self> constPtr;
        public:
          /** \todo protect against NULL Impl. */
          ResImplConnect( const std::string & name_r,
                          const Edition & edition_r,
                          const Arch & arch_r,
                          Impl_Ptr impl_r )
          : _Res( name_r, edition_r, arch_r )
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
      makeResolvableAndImpl( const std::string & name_r,
                             const Edition & edition_r,
                             const Arch & arch_r,
                             base::shared_ptr<_Impl> & impl_r )
      {
        impl_r.reset( new _Impl );
        return new
               _resobjectfactory_detail::ResImplConnect<typename _Impl::ResType>
               ( name_r, edition_r, arch_r, impl_r );
      }

    template<class _Impl>
      typename _Impl::ResType::Ptr
      makeResolvableFromImpl( const std::string & name_r,
                              const Edition & edition_r,
                              const Arch & arch_r,
                              base::shared_ptr<_Impl> impl_r )
      {
        if ( ! impl_r )
          throw ( "makeResolvableFromImpl: NULL Impl " );
        if ( impl_r->self() )
          throw ( "makeResolvableFromImpl: Impl already managed" );
        return new
               _resobjectfactory_detail::ResImplConnect<typename _Impl::ResType>
               ( name_r, edition_r, arch_r, impl_r );
      }

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_RESOBJECTFACTORY_H
