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
    // data access:

    /** selection summary (FIXME: localized) */
    Label summary() const;

    /** */
    Text description() const;

    /** selection category */
    Label category() const;

    /** selection visibility (for hidden selections) */
    bool visible() const;

    /** selection presentation order */
    Label order() const;

    std::set<std::string> suggests() const;
    std::set<std::string> recommends() const;
    std::set<std::string> install_packages( const Locale & lang = Locale("") ) const;

  protected:
    /** Ctor */
    Selection( const NVRAD & nvrad_r );
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
