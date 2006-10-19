/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/susetags/SuseTagsProductImpl.h
 *
*/
#ifndef ZYPP_DETAIL_SUSETAGS_PRODUCTIMPL_H
#define ZYPP_DETAIL_SUSETAGS_PRODUCTIMPL_H

#include <map>

#include "zypp/CheckSum.h"
#include "zypp/CapSet.h"
#include "zypp/detail/ProductImplIf.h"
#include "zypp/Source.h"
#include "zypp/TranslatedText.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace susetags
    { /////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : ProductImpl
      //
      /**
      */
      struct SuseTagsProductImpl : public zypp::detail::ProductImplIf
      {
      public:
        SuseTagsProductImpl();
        virtual ~SuseTagsProductImpl();

        virtual std::string category() const;
        virtual Label vendor() const;
        virtual TranslatedText summary() const;
        virtual Source_Ref source() const;
        virtual Url releaseNotesUrl() const;
        virtual std::list<Url> updateUrls() const;
        virtual std::list<std::string> flags() const;
        virtual TranslatedText shortName() const;
        virtual std::string distributionName() const;
        virtual Edition distributionEdition() const;

        std::string _category;

        std::string _name;
        std::string _version;
        std::string _dist_name;
        Edition     _dist_version;

        std::string _base_product;
        std::string _base_version;
        std::string _you_type;
        std::string _shortlabel;
        std::string _vendor;
        Url _release_notes_url;
        std::list<Url> _update_urls;
        std::map< std::string, std::list<std::string> > _arch;	// map of 'arch : "arch1 arch2 arch3"', arch1 being 'best', arch3 being 'noarch' (ususally)
        std::string _default_base;
        Dependencies _deps;
        std::list<std::string> _languages;
        TranslatedText _summary;
        std::string _description_dir;
        std::string _data_dir;
        std::list<std::string> _flags;
        std::string _language;
        std::string _timezone;

        std::map<std::string, CheckSum> _descr_files_checksums;
        std::map<std::string, CheckSum> _signing_keys;

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
#endif // ZYPP_DETAIL_PRODUCTIMPL_H
