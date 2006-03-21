/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/detail/SuseTagsProductImpl.cc
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
  namespace susetags
  {
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
    { return _category; }

    Label SuseTagsProductImpl::vendor() const
    { return _vendor; }

    TranslatedText SuseTagsProductImpl::summary() const
    { return _summary; }

    Source_Ref SuseTagsProductImpl::source() const
    { return _source; }
    
    Url SuseTagsProductImpl::releaseNotesUrl() const
    { return _release_notes_url; }

    std::list<std::string> SuseTagsProductImpl::flags() const
    { return _flags; }

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  }
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
