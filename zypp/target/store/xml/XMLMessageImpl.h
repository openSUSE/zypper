/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/detail/XMLMessageImpl.h
 *
*/
#ifndef ZYPP_STORE_XMLMESSAGEIMPL_H
#define ZYPP_STORE_XMLMESSAGEIMPL_H

#include "zypp/detail/MessageImplIf.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace storage
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : XMLMessageImpl
    //
    /** Class representing the message to be shown during update */
    struct XMLMessageImpl : public zypp::detail::MessageImplIf
    {
    
      /** Default ctor */
      XMLMessageImpl();
      /** Dtor */
      virtual ~XMLMessageImpl();

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
      
      /** Get the text of the message */
      virtual TranslatedText text() const;
    
      /** The text of the message */
      TranslatedText _text;
      
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
      
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace storage
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_MESSAGEIMPL_H
