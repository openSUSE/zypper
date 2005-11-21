/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/Selection.h
 *
*/
#ifndef ZYPP_SELECTION_H
#define ZYPP_SELECTION_H

#include "zypp/ResObject.h"
#include "zypp/detail/SelectionImplIf.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Selection
  //
  /** Selection interface.
  */
  class Selection : public ResObject
  {
  public:
    typedef detail::SelectionImplIf  Impl;
    typedef Selection                Self;
    typedef ResTraits<Self>          TraitsType;
    typedef TraitsType::PtrType      Ptr;
    typedef TraitsType::constPtrType constPtr;

  public:
    /** */
    // data here:

  protected:
    /** Ctor */
    Selection( const std::string & name_r,
               const Edition & edition_r,
               const Arch & arch_r );
    /** Dtor */
    virtual ~Selection();

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
#endif // ZYPP_SELECTION_H
