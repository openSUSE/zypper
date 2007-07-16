/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/SrcPackage.h
 *
*/
#ifndef ZYPP_SRCPACKAGE_H
#define ZYPP_SRCPACKAGE_H

#include "zypp/ResObject.h"
#include "zypp/detail/SrcPackageImplIf.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  DEFINE_PTR_TYPE(SrcPackage);

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : SrcPackage
  //
  /** SrcPackage interface.
  */
  class SrcPackage : public ResObject
  {

  public:
    typedef detail::SrcPackageImplIf    Impl;
    typedef SrcPackage                  Self;
    typedef ResTraits<Self>          TraitsType;
    typedef TraitsType::PtrType      Ptr;
    typedef TraitsType::constPtrType constPtr;

  public:
    /** Disk usage per directory */
    DiskUsage diskusage() const;
    
    /** location of resolvable in repo */
    OnMediaLocation location() const;
      
  protected:
    SrcPackage( const NVRAD & nvrad_r );
    /** Dtor */
    virtual ~SrcPackage();

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
#endif // ZYPP_SRCPACKAGE_H
