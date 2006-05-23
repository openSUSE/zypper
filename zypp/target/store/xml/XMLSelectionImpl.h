/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/detail/XMLSelectionImpl.h
 *
*/
#ifndef ZYPP_TARGET_XMLSELECTIONIMPL_H
#define ZYPP_TARGET_XMLSELECTIONIMPL_H

#include "zypp/detail/SelectionImplIf.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace storage
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : XMLSelectionImpl
    //
    /**
    */
    struct XMLSelectionImpl : public zypp::detail::SelectionImplIf
    {
      XMLSelectionImpl();
      virtual ~XMLSelectionImpl();

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

      /** selection category */
      Label category() const;

      /** selection visibility (for hidden selections) */
      bool visible() const;

      /** selection presentation order */
      Label order() const;
      
      const std::set<std::string> suggests() const;
      const std::set<std::string> recommends() const;
      const std::set<std::string> install_packages( const Locale & lang = Locale("") ) const;

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
      
      std::string _name;
      std::string _version;
      std::string _release;
      std::string _arch;
      std::string _order;
      std::string _category;
      bool _visible;

      std::set<std::string> _suggests;
      std::set<std::string> _recommends;
      std::set<std::string> _install_packages;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_SELECTIONIMPL_H
