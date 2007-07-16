
#ifndef ZYPP_SUSE_MEDIAVERIFIER_H
#define ZYPP_SUSE_MEDIAVERIFIER_H

#include "zypp/media/MediaManager.h"
#include "zypp/media/MediaAccess.h"

namespace zypp
{
  namespace repo
  {

    class SUSEMediaVerifier : public zypp::media::MediaVerifierBase
    {
      public:
      /** ctor */
      SUSEMediaVerifier(const std::string & vendor_r, const std::string & id_r, const media::MediaNr media_nr = 1);
      /**
        * Check if the specified attached media contains
        * the desired media number (e.g. SLES10 CD1).
        */
      virtual bool isDesiredMedia(const media::MediaAccessRef &ref);
      private:
        std::string _media_vendor;
        std::string _media_id;
        media::MediaNr _media_nr;
    };

  }
}
#endif
