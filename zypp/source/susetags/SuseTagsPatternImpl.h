/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/susetags/SuseTagsPatternImpl.h
 *
*/
#ifndef ZYPP_DETAIL_SUSETAGS_PATTERNIMPL_H
#define ZYPP_DETAIL_SUSETAGS_PATTERNIMPL_H

#include "zypp/detail/PatternImplIf.h"
#include "zypp/Source.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////
  namespace susetags
  {

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : PatternImpl
    //
    /**
    */
    struct SuseTagsPatternImpl : public zypp::detail::PatternImplIf
    {
    public:
      SuseTagsPatternImpl();
      virtual ~SuseTagsPatternImpl();

      virtual TranslatedText summary() const;
      virtual TranslatedText description() const;
      virtual TranslatedText category() const;
      virtual bool userVisible() const;
      virtual Label order() const;
      virtual Pathname icon() const;
      virtual Source_Ref source() const;
      virtual const CapSet & includes() const;
      virtual const CapSet & extends() const;

      TranslatedText _summary;
      TranslatedText _description;
      TranslatedText _category;
      bool           _visible;
      std::string    _order;
      Pathname       _icon;

      CapSet         _includes;
      CapSet         _extends;

      Source_Ref _source;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace susetags
  ///////////////////////////////////////////////////////////////////
  } // namespace source
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_PATTERNIMPL_H
