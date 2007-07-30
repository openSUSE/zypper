/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/target/store/xml/XMLScriptImpl.h
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
      virtual ByteCount downloadSize() const
      { return _downloadSize; }
      virtual bool installOnly() const
      { return _install_only; }
      virtual Date buildtime() const
      { return _build_time; }
      virtual Date installtime() const
      { return _install_time; }

      virtual std::string doScriptInlined() const;
      virtual std::string undoScriptInlined() const;

      TranslatedText _summary;
      TranslatedText _description;

      TranslatedText _install_notify;
      TranslatedText _delete_notify;
      TranslatedText _license_to_confirm;
      std::string _vendor;
      ByteCount _size;
      ByteCount _downloadSize;
      bool _install_only;
      Date _build_time;
      Date _install_time;

      std::string _doScript;
      std::string _undoScript;
   };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_SCRIPTIMPL_H
