#ifndef MEDIAHANDLERFACTORY_H
#define MEDIAHANDLERFACTORY_H

#include <zypp/Pathname.h>
#include <zypp/Url.h>
#include <memory>


namespace zypp::media {

  class MediaHandler;

  class MediaHandlerFactory
  {
  public:
    MediaHandlerFactory();
    static std::unique_ptr<MediaHandler> createHandler (const Url& o_url, const Pathname & preferred_attach_point);
  };

}


#endif // MEDIAHANDLERFACTORY_H
