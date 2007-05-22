/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp2/repository/memory/DProductImpl.cc
 *
*/
#include "zypp2/repository/memory/DProductImpl.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace repository
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace memory
    { /////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : DProductImpl::DProductImpl
      //	METHOD TYPE : Ctor
      //
      DProductImpl::DProductImpl(data::Product_Ptr ptr)
      {}

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : DProductImpl::~DProductImpl
      //	METHOD TYPE : Dtor
      //
      DProductImpl::~DProductImpl()
      {}


      std::string DProductImpl::category() const
      {
        return _category;
      }

      Label DProductImpl::vendor() const
      {
        return _vendor;
      }

      TranslatedText DProductImpl::summary() const
      {
        return _summary;
      }

      Source_Ref DProductImpl::source() const
      {
        return Source_Ref::noSource;
      }

      Url DProductImpl::releaseNotesUrl() const
      {
        return _release_notes_url;
      }

      std::list<Url> DProductImpl::updateUrls() const
      {
        return _update_urls;
      }

      std::list<Url> DProductImpl::extraUrls() const
      {
        return _extra_urls;
      }
      
      std::list<Url> DProductImpl::optionalUrls() const
      {
        return _optional_urls;
      }
      
      std::list<std::string> DProductImpl::flags() const
      {
        return _flags;
      }

      TranslatedText DProductImpl::shortName() const
      {
        return TranslatedText(_shortlabel);
      }

      std::string DProductImpl::distributionName() const
      {
        return _dist_name;
      }

      Edition DProductImpl::distributionEdition() const
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
