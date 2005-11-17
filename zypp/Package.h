/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/Package.h
 *
*/
#ifndef ZYPP_PACKAGE_H
#define ZYPP_PACKAGE_H

#include "zypp/ResObject.h"
#include "zypp/detail/PackageImplIf.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Package
  //
  /** Package interface.
  */
  class Package : public ResObject
  {
  public:
    typedef Package                         Self;
    typedef detail::PackageImplIf           Impl;
    typedef base::intrusive_ptr<Self>       Ptr;
    typedef base::intrusive_ptr<const Self> constPtr;

  public:
    /** */
    // data here:

  protected:
    /** Ctor */
    Package( const std::string & name_r,
             const Edition & edition_r,
             const Arch & arch_r );
    /** Dtor */
    virtual ~Package();

  private:
    /** Access implementation */
    virtual Impl & pimpl() = 0;
    /** Access implementation */
    virtual const Impl & pimpl() const = 0;
  };
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PACKAGE_H
