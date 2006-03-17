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
      virtual Label summary( const Locale & locale_r = Locale() ) const;
      virtual Label description( const Locale & locale_r = Locale() ) const;
      virtual Url releaseNotesUrl() const;

      std::string _category;
      std::string _vendor;
      Url _release_notes_url;
      TranslatedText _summary;
      TranslatedText _description;
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
