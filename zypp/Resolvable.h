/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/Resolvable.h
 *
*/
#ifndef ZYPP_RESOLVABLE_H
#define ZYPP_RESOLVABLE_H

#include <iosfwd>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////
    DEFINE_PTR_TYPE(ResolvableImpl)
    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  DEFINE_PTR_TYPE(Resolvable)

  class ResKind;
  class ResName;
  class ResEdition;
  class ResArch;

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Resolvable
  //
  /** */
  class Resolvable : public base::ReferenceCounted, private base::NonCopyable
  {
  public:
    /** ctor */
    Resolvable( detail::ResolvableImplPtr impl_r );
    /** Dtor */
    virtual ~Resolvable();

  public:
    /**  */
    const ResKind & kind() const;
    /**  */
    const ResName & name() const;
    /**  */
    const ResEdition & edition() const;
    /**  */
    const ResArch & arch() const;

  private:
    /** Pointer to implementation */
    detail::ResolvableImplPtr _pimpl;
  public:
    /** Avoid a bunch of friend decl. */
    detail::constResolvableImplPtr sayFriend() const;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates Resolvable Stream output */
  extern std::ostream & operator<<( std::ostream & str, const Resolvable & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_RESOLVABLE_H
