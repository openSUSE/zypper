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
      typedef sat::ArrayAttr<IdString,IdString> NameList;
      typedef sat::SolvableSet                  Contents;

    public:
      /** */
      bool isDefault() const;
      /** */
      bool userVisible() const;
      /** */
      std::string category( const Locale & lang_r = Locale() ) const;
      /** */
      Pathname icon() const;
      /** */
      Pathname script() const;
      /** */
      std::string order() const;

    public:
      /** Ui hint: included patterns. */
      NameList includes() const;

      /** Ui hint: patterns this one extends. */
      NameList extends() const;

      /** Ui hint: Required Packages. */
      Contents core() const;

      /** Ui hint: Dependent packages.
       * This also includes recommended and suugested packages.
      */
      Contents depends() const;

      /** The collection of packages associated with this pattern.
       * This also evaluates the patterns includes/extends relation.
       */
      Contents contents() const;

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
