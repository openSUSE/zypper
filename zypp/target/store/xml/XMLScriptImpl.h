/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/detail/XMLScriptImpl.h
 *
*/
#ifndef ZYPP_STORE_XMLSCRIPTIMPL_H
#define ZYPP_STORE_XMLSCRIPTIMPL_H

#include "zypp/TmpPath.h"
#include "zypp/detail/ScriptImplIf.h"

using namespace zypp::filesystem;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace storage
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : XMLScriptImpl
    //
    /** Class representing an update script */
    struct XMLScriptImpl : public zypp::detail::ScriptImplIf
    {
      /** Default ctor */
      XMLScriptImpl();
      /** Dtor */
      ~XMLScriptImpl();

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
      
      /** Get the script to perform the change */
      Pathname do_script() const;
      /** Get the script to undo the change */
      Pathname undo_script() const;
      /** Check whether script to undo the change is available */
      virtual bool undo_available() const;
      
      mutable shared_ptr<filesystem::TmpFile> _do_script;
      mutable shared_ptr<filesystem::TmpFile> _undo_script;      

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
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_SCRIPTIMPL_H
