/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/repository/memory/PatternImpl.h
 *
*/
#ifndef ZYPP_DETAIL_MEMORY_PATTERNIMPL_H
#define ZYPP_DETAIL_MEMORY_PATTERNIMPL_H

#include "zypp/detail/PatternImplIf.h"
#include "zypp/data/ResolvableData.h"
#include "zypp/Repository.h"
#include "zypp/repo/memory/RepoImpl.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace repo
  { /////////////////////////////////////////////////////////////////
    namespace memory
    {

      struct PatternImpl : public zypp::detail::PatternImplIf
      {
      public:
        PatternImpl( memory::RepoImpl::Ptr repo, data::Pattern_Ptr ptr);
        virtual ~PatternImpl();

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
        
        virtual TranslatedText category() const;
        virtual bool userVisible() const;
        virtual Label order() const;
        virtual Pathname icon() const;
      private:
        
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
        
        // Pattern
        TranslatedText _category;
        bool           _visible;
        std::string    _order;
        Pathname       _icon;
      };
      ///////////////////////////////////////////////////////////////////

      /////////////////////////////////////////////////////////////////
    } // namespace memory
    ///////////////////////////////////////////////////////////////////
  } // namespace repository
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_PATTERNIMPL_H
