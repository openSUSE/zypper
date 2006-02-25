/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/Atom.h
 *
*/
#ifndef ZYPP_ATOM_H
#define ZYPP_ATOM_H

#include "zypp/ResObject.h"
#include "zypp/detail/AtomImplIf.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Atom
  //
  /** Class representing the message to be shown during update.
  */
  class Atom : public ResObject
  {
  public:
    typedef detail::AtomImplIf       Impl;
    typedef Atom                     Self;
    typedef ResTraits<Self>          TraitsType;
    typedef TraitsType::PtrType      Ptr;
    typedef TraitsType::constPtrType constPtr;

  protected:
    Atom( const NVRAD & nvrad_r );
    /** Dtor */
    virtual ~Atom();

  private:
    /** Access implementation */
    virtual Impl & pimpl() = 0;
    /** Access implementation */
    virtual const Impl & pimpl() const = 0;
  };

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_ATOM_H
