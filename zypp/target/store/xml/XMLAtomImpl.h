/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/target/store/xml/XMLAtomImpl.h
 *
*/
#ifndef ZYPP_TARGET_XMLSTORE_ATOMIMPL_H
#define ZYPP_TARGET_XMLSTORE_ATOMIMPL_H

#include "zypp/source/SourceImpl.h"
#include "zypp/detail/AtomImpl.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
    namespace storage
    { //////////////////////////////////////////////////////////////

      
      /** Class representing a Atom
      */
      class XMLAtomImpl : public detail::AtomImplIf
      {
      public:
        /** Default ctor */
        XMLAtomImpl();
      private:
	
      public:
	//Source_Ref source() const;
        
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
    } // namespace storage
    /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_YUM_YUMATOMIMPL_H
