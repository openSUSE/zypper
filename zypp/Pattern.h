/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/Pattern.h
 *
*/
#ifndef ZYPP_PATTERN_H
#define ZYPP_PATTERN_H

#include "zypp/ResObject.h"
#include "zypp/Pathname.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  DEFINE_PTR_TYPE(Pattern);

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Pattern
  //
  /** Pattern interface.
  */
  class Pattern : public ResObject
  {
  public:
    typedef Pattern                  Self;
    typedef ResTraits<Self>          TraitsType;
    typedef TraitsType::PtrType      Ptr;
    typedef TraitsType::constPtrType constPtr;

  public:
    /** */
    bool isDefault() const;
    /** */
    bool userVisible() const;
    /** */
    std::string category() const;
    /** */
    Pathname icon() const;
    /** */
    Pathname script() const;
    /** */
    Label order() const;

    /** \deprecated AFAIK unused old Selection interface method. */
    std::set<std::string> install_packages( const Locale & lang = Locale("") ) const ZYPP_DEPRECATED;

    /** Ui hint. */
    const Capabilities & includes() const;
    /** Ui hint. */
    const Capabilities & extends() const;

  protected:
    friend Ptr make<Self>( const sat::Solvable & solvable_r );
    /** Ctor */
    Pattern( const sat::Solvable & solvable_r );
    /** Dtor */
    virtual ~Pattern();
  };
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PATTERN_H
