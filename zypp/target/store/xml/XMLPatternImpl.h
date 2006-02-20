/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/detail/XMLPatternImpl.h
 *
*/
#ifndef ZYPP_STORAGE_XMLPATTERNIMPL_H
#define ZYPP_STORAGE_XMLPATTERNIMPL_H

#include "zypp/detail/PatternImplIf.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace storage
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : XMLPatternImpl
    //
    /**
    */
    struct XMLPatternImpl : public zypp::detail::PatternImplIf
    {
      XMLPatternImpl();
      virtual ~XMLPatternImpl();

      virtual bool userVisible() const;
      virtual TranslatedText summary() const;
      virtual TranslatedText description() const;
      virtual Text insnotify() const;
      virtual Text delnotify() const;
      virtual bool providesSources() const;
      virtual Label instSrcLabel() const;
      virtual Vendor instSrcVendor() const;
      virtual ByteCount size() const;
      virtual bool isDefault() const;
      virtual TranslatedText category() const;
      virtual Pathname icon() const;
      virtual Pathname script() const;

      bool _user_visible;
      TranslatedText _summary;
      TranslatedText _description;
      bool _default;
      TranslatedText _category;
      Pathname _icon;
      Pathname _script;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace storage
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_PATTERNIMPL_H
