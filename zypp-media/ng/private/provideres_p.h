#ifndef ZYPP_MEDIA_PRIVATE_PROVIDERES_P_H
#define ZYPP_MEDIA_PRIVATE_PROVIDERES_P_H

#include <zypp-core/ManagedFile.h>
#include <zypp-core/Url.h>
#include <zypp-media/ng/Provide>
#include <zypp-media/ng/HeaderValueMap>
#include "providefwd_p.h"

namespace zyppng {
  /*!
   * \internal
   * The internal shared data structure for \sa ProvideRes objects.
   */
  struct ProvideResourceData {
    zyppng::Provide::MediaHandle _mediaHandle;
    zypp::ManagedFile _myFile;
    zypp::Url _resourceUrl; //< The resource where the file was provided from
    HeaderValueMap _responseHeaders; //< The response headers
  };
}

#endif // ZYPP_MEDIA_PRIVATE_PROVIDERES_P_H
