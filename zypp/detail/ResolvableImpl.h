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

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Resolvable::Impl
  //
  /** Implementation of Resovable
   * \todo Assert \c deps provide 'name=edition'.
  */
  struct Resolvable::Impl
  {
    /** Ctor */
    Impl( const Kind & kind_r,
          const std::string & name_r,
          const Edition & edition_r,
          const Arch & arch_r )
    : _kind( kind_r )
    , _name( name_r )
    , _edition( edition_r )
    , _arch( arch_r )
    {}

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

    /** Set Dependencies.
     * \todo Check whether we can allow changes after final construction
    */
    void setDeps( const Dependencies & val_r )
    { _deps = val_r; }

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
