/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include <fstream>
#include "zypp/repo/SUSEMediaVerifier.h"

using namespace std;

namespace zypp
{
namespace repo
{

SUSEMediaVerifier::SUSEMediaVerifier(const std::string & vendor_r,
                                     const std::string & id_r,
                                     const media::MediaNr media_nr)
   : _media_vendor(vendor_r)
    , _media_id(id_r)
    , _media_nr(media_nr)
{}

SUSEMediaVerifier::SUSEMediaVerifier( int media_nr, const Pathname &path_r )
  : _media_nr(media_nr)
{
  std::ifstream str(path_r.asString().c_str());
  std::string vendor;
  std::string id;
  
  if ( str )
  {
    getline(str, _media_vendor);
    getline(str, _media_id);
  }
  else
  {
    ZYPP_THROW(Exception("Can't setup media verifier using file: '"
        + path_r.asString() + "'"));
  }
}

bool SUSEMediaVerifier::isDesiredMedia(const media::MediaAccessRef &ref)
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

}
}

