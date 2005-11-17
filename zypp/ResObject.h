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
#include "zypp/detail/ResObjectImplIf.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : ResObject
  //
  /** Interface base for resolvable objects (common data).
  */
  class ResObject : public Resolvable
  {
  public:
    typedef ResObject                       Self;
    typedef detail::ResObjectImplIf         Impl;
    typedef base::intrusive_ptr<Self>       Ptr;
    typedef base::intrusive_ptr<const Self> constPtr;

  public:
    /** */
    line summary() const;
    /** */
    text description() const;

  protected:
    /** Ctor */
    ResObject( const ResKind & kind_r,
               const std::string & name_r,
               const Edition & edition_r,
               const Arch & arch_r );
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
