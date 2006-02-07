/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ResObject.h
 *
*/
#ifndef ZYPP_RESOBJECT_H
#define ZYPP_RESOBJECT_H

#include "zypp/Resolvable.h"
#include "zypp/TranslatedText.h"
//#include "zypp/detail/ResObjectImplIf.h"
#include "zypp/NeedAType.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  namespace detail {
    class ResObjectImplIf;
  }
  class Source_Ref;
  class ByteCount;

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : ResObject
  //
  /** Interface base for resolvable objects (common data).
  */
  class ResObject : public Resolvable
  {
  public:
    typedef detail::ResObjectImplIf  Impl;
    typedef ResObject                Self;
    typedef ResTraits<Self>          TraitsType;
    typedef TraitsType::PtrType      Ptr;
    typedef TraitsType::constPtrType constPtr;

  public:
    /** */
    Text summary() const;

    /** */
    Text description() const;

    /** */
    Text insnotify() const;

    /** */
    Text delnotify() const;

    /** */
    ByteCount size() const;

    /** */
    Source_Ref source() const;

    /** */
    ZmdId zmdid () const;

  protected:
    /** Ctor */
    ResObject( const Kind & kind_r,
               const NVRAD & nvrad_r );
    /** Dtor */
    virtual ~ResObject();

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
#endif // ZYPP_RESOBJECT_H
