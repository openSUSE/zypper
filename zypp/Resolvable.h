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

#include "zypp/base/PtrTypes.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class ResKind;
  class ResName;
  class ResEdition;
  class ResArch;

  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////
    /** Hides implementation */
    class ResolvableImpl;
    typedef base::shared_ptr<ResolvableImpl> ResolvableImplPtr;
    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Resolvable
  //
  /** */
  class Resolvable
  {
  public:
    /** Default ctor */
    Resolvable();
    /** ctor */
    Resolvable( detail::ResolvableImplPtr impl_r );
    /** Dtor */
    ~Resolvable();
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
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates Resolvable Stream output */
  extern std::ostream & operator<<( std::ostream & str, const Resolvable & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_RESOLVABLE_H
