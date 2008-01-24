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

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  DEFINE_PTR_TYPE(Selection);

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Selection
  //
  /** Selection interface.
  */
  class Selection : public ResObject
  {
  public:
    typedef Selection                Self;
    typedef ResTraits<Self>          TraitsType;
    typedef TraitsType::PtrType      Ptr;
    typedef TraitsType::constPtrType constPtr;

  public:
    /** selection category */
    Label category() const;
    inline bool isBase() const
    { return category() == "baseconf"; }

    /** selection visibility (for hidden selections) */
    bool visible() const;

    /** selection presentation order */
    Label order() const;

    const std::set<std::string> install_packages( const Locale & lang = Locale("") ) const;

  protected:
    friend Ptr make<Self>( const sat::Solvable & solvable_r );
    /** Ctor */
    Selection( const sat::Solvable & solvable_r );
    /** Dtor */
    virtual ~Selection();
  };
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SELECTION_H
