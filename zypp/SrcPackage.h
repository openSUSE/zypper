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
    typedef SrcPackage               Self;
    typedef ResTraits<Self>          TraitsType;
    typedef TraitsType::PtrType      Ptr;
    typedef TraitsType::constPtrType constPtr;

  public:
    /** The type of the source rpm ("src" or "nosrc"). */
    std::string sourcePkgType() const;

    /** location of resolvable in repo */
    OnMediaLocation location() const;

  protected:
    friend Ptr make<Self>( const sat::Solvable & solvable_r );
    /** Ctor */
    SrcPackage( const sat::Solvable & solvable_r );
    /** Dtor */
    virtual ~SrcPackage();
  };
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SRCPACKAGE_H
