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

#include "zypp/Resolvable.h"
#include "zypp/CapFactory.h"
#include "zypp/NVRAD.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Resolvable::Impl
  //
  /** Implementation of Resovable
   * \invariant \c provides <tt>name = edition</tt>
   * \invariant \c prerequires is a subset of \c requires
  */
  struct Resolvable::Impl
  {
    /** Ctor */
    Impl( const Kind & kind_r, const NVRAD & nvrad_r );

  public:
    /**  */
    const Kind & kind() const
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

    /** \name Deprecated. */
    //@{
    void injectProvides( const Capability & cap_r )
    { _deps[Dep::PROVIDES].insert( cap_r ); }
    void injectRequires( const Capability & cap_r )
    { _deps[Dep::REQUIRES].insert( cap_r ); }
    //@}

  private:
    /**  */
    Kind _kind;
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

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_RESOLVABLEIMPL_H
