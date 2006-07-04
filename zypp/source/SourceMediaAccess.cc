/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/SourceMediaAccess.cc
 *
*/
#include <iostream>
#include <fstream>

#include "zypp/base/LogTools.h"
#include "zypp/source/SourceMediaAccess.h"
//#include "zypp/source/SourceMediaAccessReportReceivers.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace source
{ /////////////////////////////////////////////////////////////////


  SourceMediaAccess::SourceMediaAccess(  const Url &url, const Pathname &path, std::vector<media::MediaVerifierRef> verifiers )
      : _url(url),
        _path(path)
  {
    
  }
  
  SourceMediaAccess::~SourceMediaAccess()
  {
  }
    
  

//     media::MediaVerifierRef SourceMediaAccess::verifier(unsigned media_nr)
//     { return media::MediaVerifierRef(new media::NoVerifier()); }

  SourceMediaVerifier::SourceMediaVerifier(const std::string & vendor_r, const std::string & id_r, const media::MediaNr media_nr)
    : _media_vendor(vendor_r)
      , _media_id(id_r)
      , _media_nr(media_nr)
  {}

  bool SourceMediaVerifier::isDesiredMedia(const media::MediaAccessRef &ref)
  {
    if (_media_vendor.empty() || _media_id.empty())
      return true;

      Pathname media_file = "/media." + str::numstring(_media_nr) + "/media";
      ref->provideFile (media_file);
      media_file = ref->localPath(media_file);
      std::ifstream str(media_file.asString().c_str());
      std::string vendor;
      std::string id;

#warning check the stream status
      getline(str, vendor);
      getline(str, id);

      return (vendor == _media_vendor && id == _media_id );
  }


/////////////////////////////////////////////////////////////////
} // namespace source
///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
