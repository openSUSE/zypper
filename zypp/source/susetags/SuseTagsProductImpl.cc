/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/susetags/SuseTagsProductImpl.cc
 *
*/
#include "zypp/source/susetags/SuseTagsProductImpl.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace susetags
    { /////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : SuseTagsProductImpl::SuseTagsProductImpl
      //	METHOD TYPE : Ctor
      //
      SuseTagsProductImpl::SuseTagsProductImpl()
      {}

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : SuseTagsProductImpl::~SuseTagsProductImpl
      //	METHOD TYPE : Dtor
      //
      SuseTagsProductImpl::~SuseTagsProductImpl()
      {}


      std::string SuseTagsProductImpl::category() const
      {
        return _category;
      }

      Label SuseTagsProductImpl::vendor() const
      {
        return _vendor;
      }

      TranslatedText SuseTagsProductImpl::summary() const
      {
        return _summary;
      }

      Source_Ref SuseTagsProductImpl::source() const
      {
        return _source;
      }

      Url SuseTagsProductImpl::releaseNotesUrl() const
      {
        return _release_notes_url;
      }

      std::list<Url> SuseTagsProductImpl::updateUrls() const
      {
        return _update_urls;
      }

      std::list<Url> SuseTagsProductImpl::extraUrls() const
      {
        return _extra_urls;
      }
      
      std::list<Url> SuseTagsProductImpl::optionalUrls() const
      {
        return _optional_urls;
      }
      
      std::list<std::string> SuseTagsProductImpl::flags() const
      {
        return _flags;
      }

      TranslatedText SuseTagsProductImpl::shortName() const
      {
        return TranslatedText(_shortlabel);
      }

      std::string SuseTagsProductImpl::distributionName() const
      {
        return _dist_name;
      }

      Edition SuseTagsProductImpl::distributionEdition() const
      {
        return _dist_version;
      }

      /////////////////////////////////////////////////////////////////
    } // namespace susetags
    ///////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
