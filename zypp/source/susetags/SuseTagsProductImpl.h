/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/detail/ProductImpl.h
 *
*/
#ifndef ZYPP_DETAIL_SUSETAGS_PRODUCTIMPL_H
#define ZYPP_DETAIL_SUSETAGS_PRODUCTIMPL_H

#include "zypp/detail/ProductImplIf.h"

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
      virtual Label displayName() const;

      //std::string _vendor;
      std::string _category;
      std::string _displayName;

      std::string _name;
      std::string _version;
      std::string _dist;
      std::string _dist_version;
      std::string _base_product;
      std::string _base_version;
      std::string _you_type;
      std::string _you_path;
      std::string _you_url;
      std::string _vendor;
      std::string _release_notes_url;
      std::map< std::string, std::list<std::string> > _arch;
      std::string _default_base;
      std::list<std::string> _requires;
      std::list<std::string> _languages;
      TranslatedText _label;
      std::string _description_dir;
      std::string _data_dir;
      std::list<std::string> _flags;
      std::string _language;
      std::string _timezone;
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
