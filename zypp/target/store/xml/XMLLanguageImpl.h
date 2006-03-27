/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/detail/XMLLanguageImpl.h
 *
*/
#ifndef ZYPP_STORAGE_XMLLANGUAGEIMPL_H
#define ZYPP_STORAGE_XMLLANGUAGEIMPL_H

#include "zypp/detail/LanguageImplIf.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace storage
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : XMLLanguageImpl
    //
    /**
    */
    struct XMLLanguageImpl : public zypp::detail::LanguageImplIf
    {
      XMLLanguageImpl();
      virtual ~XMLLanguageImpl();

      virtual TranslatedText summary() const;
      virtual TranslatedText description() const;
      
      TranslatedText _summary;
      TranslatedText _description;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace storage
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_PATTERNIMPL_H
