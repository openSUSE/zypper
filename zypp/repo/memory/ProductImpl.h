/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/repo/memory/ProductImpl.h
 *
*/
#ifndef ZYPP_DETAIL_MEMORY_PRODUCTIMPL_H
#define ZYPP_DETAIL_MEMORY_PRODUCTIMPL_H

#include <map>

#include "zypp/CheckSum.h"
#include "zypp/CapSet.h"
#include "zypp/detail/ProductImplIf.h"
#include "zypp/data/ResolvableData.h"
#include "zypp/TranslatedText.h"

#include "zypp/repo/memory/RepoImpl.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace repo
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace memory
    { /////////////////////////////////////////////////////////////////

      struct ProductImpl : public zypp::detail::ProductImplIf
      {
      public:
        ProductImpl( memory::RepoImpl::Ptr repo, data::Product_Ptr ptr);
        virtual ~ProductImpl();

        virtual Repository repository() const;

        virtual TranslatedText summary() const;
        virtual TranslatedText description() const;
        virtual TranslatedText insnotify() const;
        virtual TranslatedText delnotify() const;
        virtual TranslatedText licenseToConfirm() const;
        virtual Vendor vendor() const;
        virtual ByteCount size() const;
        virtual bool installOnly() const;
        virtual Date buildtime() const;
        virtual Date installtime() const;

        virtual std::string type() const;
        virtual Url releaseNotesUrl() const;
        virtual std::list<Url> updateUrls() const;
        virtual std::list<Url> extraUrls() const;
        virtual std::list<Url> optionalUrls() const;
        virtual std::list<std::string> flags() const;
        virtual TranslatedText shortName() const;
        virtual std::string distributionName() const;
        virtual Edition distributionEdition() const;

        repo::memory::RepoImpl::Ptr _repository;

        //ResObject
        TranslatedText _summary;
        TranslatedText _description;
        TranslatedText _insnotify;
        TranslatedText _delnotify;
        TranslatedText _license_to_confirm;
        Vendor _vendor;
        ByteCount _size;
        bool _install_only;
        Date _buildtime;
        Date _installtime;

        std::string _type;
        std::string _dist_name;
        Edition     _dist_version;
        std::string _base_product;
        std::string _base_version;
        std::string _you_type;
        std::string _shortlabel;
        Url _release_notes_url;

        std::list<Url> _update_urls;
        std::list<Url> _extra_urls;
        std::list<Url> _optional_urls;

        std::string _default_base;
        std::list<std::string> _flags;
      };
      ///////////////////////////////////////////////////////////////////

      /////////////////////////////////////////////////////////////////
    } // namespace memory
    ///////////////////////////////////////////////////////////////////
  } // namespace repository
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_PRODUCTIMPL_H
