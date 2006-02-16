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

#include "zypp/detail/ResObjectImplIf.h"

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
    /** Connect to an implementation (internal).
     * \note This is not to be exposed in some public interface!
     * ImplConnect::resimpl takes a _Ptr to a resolvable as argument, and returns
     * a _Ptr to it's implementation class:
     * \code
     * // ResObject::Ptr      -> detail::ResImplTraits<ResObject::Impl>::Ptr
     * // ResObject::constPtr -> detail::ResImplTraits<ResObject::Impl>::constPtr
     * // Package::Ptr        -> detail::ResImplTraits<Package::Impl>::Ptr
     * // Package::constPtr   -> detail::ResImplTraits<Package::Impl>::constPtr
     *
     * ResObject::constPtr ptr;
     * detail::ResImplTraits<Package::Impl>::constPtr implPtr;
     *
     * implPtr = detail::ImplConnect::resimpl( asKind<Package>(ptr) );
     *
     * // implPtr will be NULL, if ptr is NULL or does not refer to a Package.
     * \endcode
     * Basically makes a ResObjectImplIf
    */
    struct ImplConnect
    {
      template<class _Res>
        static typename ResImplTraits<typename _Res::Impl>::Ptr resimpl( const intrusive_ptr<_Res> & obj )
        { return dynamic_pointer_cast<typename _Res::Impl>(getImpl( obj )); }

      template<class _Res>
        static typename ResImplTraits<typename _Res::Impl>::constPtr resimpl( const intrusive_ptr<const _Res> & obj )
        { return dynamic_pointer_cast<const typename _Res::Impl>(getConstImpl( obj )); }

    private:
      static ResImplTraits<ResObject::Impl>::Ptr getImpl( const ResObject::Ptr & obj )
      { return( obj ? &obj->pimpl() : NULL ); }

      static ResImplTraits<ResObject::Impl>::constPtr getConstImpl( const ResObject::constPtr & obj )
      { return( obj ? &obj->pimpl() : NULL ); }
     };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_IMPLCONNECT_H
