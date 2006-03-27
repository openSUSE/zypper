/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/SystemResObject.h
 *
*/
#ifndef ZYPP_SYSTEMRESOBJECT_H
#define ZYPP_SYSTEMRESOBJECT_H

#include "zypp/ResObject.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class SystemResObject;

  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //  CLASS NAME : SystemResObjectImplIf
    //
    /** Abstract SystemResObject implementation interface.
    */
    class SystemResObjectImplIf : public ResObjectImplIf
    {
    public:
      typedef SystemResObject ResType;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : SystemResObject
  //
  /** */
  class SystemResObject : public ResObject
  {
  public:
    typedef detail::SystemResObjectImplIf Impl;
    typedef SystemResObject               Self;
    typedef ResTraits<Self>               TraitsType;
    typedef TraitsType::PtrType           Ptr;
    typedef TraitsType::constPtrType      constPtr;

  public:
    /** Default SystemResObject instance. */
    static Ptr instance();

  protected:
    /** Ctor */
    SystemResObject( const NVRAD & nvrad_r );
    /** Dtor */
    virtual ~SystemResObject();

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
#endif // ZYPP_SYSTEMRESOBJECT_H
