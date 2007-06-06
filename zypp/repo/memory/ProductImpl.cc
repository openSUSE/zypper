/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/repo/memory/ProductImpl.cc
 *
*/
#include "zypp/repo/memory/ProductImpl.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace repo
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace memory
    { /////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : ProductImpl::ProductImpl
      //	METHOD TYPE : Ctor
      //
      ProductImpl::ProductImpl(data::Product_Ptr ptr)
      {}

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : ProductImpl::~ProductImpl
      //	METHOD TYPE : Dtor
      //
      ProductImpl::~ProductImpl()
      {}


      std::string ProductImpl::category() const
      {
        return _category;
      }

      Label ProductImpl::vendor() const
      {
        return _vendor;
      }

      TranslatedText ProductImpl::summary() const
      {
        return _summary;
      }

      Source_Ref ProductImpl::source() const
      {
        return Source_Ref::noSource;
      }

      Url ProductImpl::releaseNotesUrl() const
      {
        return _release_notes_url;
      }

      std::list<Url> ProductImpl::updateUrls() const
      {
        return _update_urls;
      }

      std::list<Url> ProductImpl::extraUrls() const
      {
        return _extra_urls;
      }
      
      std::list<Url> ProductImpl::optionalUrls() const
      {
        return _optional_urls;
      }
      
      std::list<std::string> ProductImpl::flags() const
      {
        return _flags;
      }

      TranslatedText ProductImpl::shortName() const
      {
        return TranslatedText(_shortlabel);
      }

      std::string ProductImpl::distributionName() const
      {
        return _dist_name;
      }

      Edition ProductImpl::distributionEdition() const
      {
        return _dist_version;
      }

      /////////////////////////////////////////////////////////////////
    } // namespace memory
    ///////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////
  } // namespace repository
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
