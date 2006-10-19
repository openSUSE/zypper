/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/target/store/xml/XMLProductImpl.h
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

      virtual TranslatedText summary() const
      { return _summary; }
      virtual TranslatedText description() const
      { return _description; }
      virtual TranslatedText insnotify() const
      { return _install_notify; }
      virtual TranslatedText delnotify() const
      { return _delete_notify; }
      virtual TranslatedText licenseToConfirm() const
      { return _license_to_confirm; }
      virtual Vendor vendor() const
      { return _vendor; }
      virtual ByteCount size() const
      { return _size; }
      virtual ByteCount archivesize() const
      { return _archive_size; }
      virtual unsigned sourceMediaNr() const
      { return 0; }
      virtual bool installOnly() const
      { return _install_only; }
      virtual Date buildtime() const
      { return _build_time; }
      virtual Date installtime() const
      { return _install_time; }

      virtual std::string category() const;
      virtual TranslatedText shortName() const;
      virtual Url releaseNotesUrl() const;
      virtual std::list<Url> updateUrls() const;
      virtual std::list<std::string> flags() const;
      virtual std::string distributionName() const;
      virtual Edition distributionEdition() const;

      std::string _category;
      Url _release_notes_url;
      std::list<Url> _update_urls;
      std::list<std::string> _flags;

      TranslatedText _summary;
      TranslatedText _description;

      TranslatedText _install_notify;
      TranslatedText _delete_notify;
      TranslatedText _license_to_confirm;
      std::string _vendor;
      ByteCount _size;
      ByteCount _archive_size;
      bool _install_only;
      Date _build_time;
      Date _install_time;
      TranslatedText _short_name;
      std::string _dist_name;
      Edition     _dist_version;

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
