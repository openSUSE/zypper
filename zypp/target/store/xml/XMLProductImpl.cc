/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/target/store/xml/XMLProductImpl.cc
 *
*/

#include "zypp/target/store/xml/XMLProductImpl.h"
#include "zypp/base/Logger.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace storage
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : XMLProductImpl
    //
    ///////////////////////////////////////////////////////////////////

    /** Default ctor */
    XMLProductImpl::XMLProductImpl()
    {}
    /** Dtor */
    XMLProductImpl::~XMLProductImpl()
    {}

    std::string XMLProductImpl::type() const
    { return _type; }

    TranslatedText XMLProductImpl::shortName() const
    { return _short_name; }

    Url XMLProductImpl::releaseNotesUrl() const
    { return _release_notes_url; }

    std::list<Url> XMLProductImpl::updateUrls() const
    { return _update_urls; }

    std::list<Url> XMLProductImpl::extraUrls() const
    { return _extra_urls; }

    std::list<Url> XMLProductImpl::optionalUrls() const
    { return _optional_urls; }

    std::list<std::string> XMLProductImpl::flags() const
    { return _flags; }

    std::string XMLProductImpl::distributionName() const
    { return _dist_name; }

    Edition XMLProductImpl::distributionEdition() const
    { return _dist_version; }

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
