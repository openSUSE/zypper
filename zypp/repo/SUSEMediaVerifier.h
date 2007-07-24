/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_SUSE_MEDIAVERIFIER_H
#define ZYPP_SUSE_MEDIAVERIFIER_H

#include "zypp/media/MediaManager.h"
#include "zypp/media/MediaAccess.h"

namespace zypp
{
  namespace repo
  {

    /**
     * \short Implementation of the traditional SUSE media verifier
     */
    class SUSEMediaVerifier : public zypp::media::MediaVerifierBase
    {
      public:
      /**
       * \short create a verifier from attributes
       *
       * Creates a verifier for the media using
       * the attributes
       *
       * \param vendor_r i.e. "SUSE Linux Products GmbH"
       * \param id_r i.e. "20070718164719"
       * \param media_nr media number
       */
      SUSEMediaVerifier(const std::string & vendor_r,
                        const std::string & id_r,
                        const media::MediaNr media_nr = 1);
      
      /**
       * \short creates a verifier from a media file
       *
       * \param path_r Path to media.1/media kind file
       */
      SUSEMediaVerifier( int media_nr, const Pathname &path_r );
      
      /**
        * \short Check if it is the desider media
        *
        * Check if the specified attached media contains
        * the desired media number (e.g. SLES10 CD1).
        *
        * Reimplementation of virtual function, will be
        * called by the component verifying the media.
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
