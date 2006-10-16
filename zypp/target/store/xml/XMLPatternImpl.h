/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/target/store/xml/XMLPatternImpl.h
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
      
      virtual bool userVisible() const;
      virtual bool isDefault() const;
      virtual TranslatedText category() const;
      virtual Pathname icon() const;
      virtual Pathname script() const;

      bool _user_visible;
      
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
