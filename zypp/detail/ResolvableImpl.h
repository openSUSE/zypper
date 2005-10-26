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

#include <string>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"

#include "zypp/ResKind.h"
#include "zypp/Edition.h"
#include "zypp/Arch.h"
#include "zypp/Dependencies.h"

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
      /** Ctor */
      ResolvableImpl( const ResKind & kind_r,
                      const std::string & name_r,
                      const Edition & edition_r,
                      const Arch & arch_r );
      /** Dtor */
      virtual ~ResolvableImpl();

    public:
      /**  */
      const ResKind & kind() const
      { return _kind; }
      /**  */
      const std::string & name() const
      { return _name; }
      /**  */
      const Edition & edition() const
      { return _edition; }
      /**  */
      const Arch & arch() const
      { return _arch; }
      /**  */
      const Dependencies & deps() const
      { return _deps; }

      /** Set Dependencies.
       * \todo Check whether we can allow changes after final construction
      */
      void setDeps( const Dependencies & val_r )
      { _deps = val_r; }

    private:
      /**  */
      ResKind _kind;
      /**  */
      std::string _name;
      /**  */
      Edition _edition;
      /**  */
      Arch _arch;
      /**  */
      Dependencies _deps;
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
