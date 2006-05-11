/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/detail/ProductImpl.h
 *
*/
#ifndef ZYPP_STORAGE_XMLPRODUCTIMPL_H
#define ZYPP_STORAGE_XMLPRODUCTIMPL_H

#include "zypp/Source.h"
#include "zypp/detail/ProductImplIf.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace storage
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ProductImpl
    //
    /** */
    struct XMLProductImpl : public zypp::detail::ProductImplIf
    {
      XMLProductImpl();
      ~XMLProductImpl();

      virtual std::string category() const;
      virtual Label vendor() const;
      virtual TranslatedText summary() const;
      virtual TranslatedText shortName() const;
      virtual TranslatedText description() const;
      virtual Url releaseNotesUrl() const;
      virtual std::list<Url> updateUrls() const;
      virtual std::list<std::string> flags() const;

      std::string _category;
      std::string _vendor;
      Url _release_notes_url;
      std::list<Url> _update_urls;
      std::list<std::string> _flags;
      TranslatedText _summary;
      TranslatedText _description;
      TranslatedText _short_name;
      Source_Ref _source;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace storage
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_PRODUCTIMPL_H
