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
#include <string>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"

#include "zypp/ResolvableFwd.h"

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

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Resolvable
  //
  /**
   * \todo Solve ResKind problems via traits template?
  */
  class Resolvable : public base::ReferenceCounted, private base::NonCopyable
  {
  public:
    /** Ctor */
    Resolvable( detail::ResolvableImplPtr impl_r );
    /** Dtor */
    virtual ~Resolvable();

  public:
    /**  */
    const ResKind & kind() const;
    /**  */
    const std::string & name() const;
    /**  */
    const Edition & edition() const;
    /**  */
    const Arch & arch() const;
    /**  */
    const Dependencies & deps() const;
    /** */
    void setDeps( const Dependencies & val_r );

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
