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

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"

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
    DEFINE_PTR_TYPE(ResolvableImpl)

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ResolvableImpl
    //
    /** */
    class ResolvableImpl : public base::ReferenceCounted, private base::NonCopyable
    {
    public:
      /** ctor */
      ResolvableImpl( const ResKind & kind_r,
                      const ResName & name_r,
                      const ResEdition & edition_r,
                      const ResArch & arch_r );
      /** Dtor */
      virtual ~ResolvableImpl();

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
      /**  */
      ResKind    _kind;
      /**  */
      ResName    _name;
      /**  */
      ResEdition _edition;
      /**  */
      ResArch    _arch;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates ResolvableImpl Stream output */
    extern std::ostream & operator<<( std::ostream & str, const ResolvableImpl & obj );

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_RESOLVABLEIMPL_H
