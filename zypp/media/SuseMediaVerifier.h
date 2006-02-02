/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file       zypp/parser/media/SuseMediaVerifier.h
 *
*/
#ifndef ZYPP_MEDIA_SUSEMEDIAVERIFIER_H
#define ZYPP_MEDIA_SUSEMEDIAVERIFIER_H

#include <zypp/media/MediaManager.h>
#include <zypp/base/String.h>
#include <zypp/base/Logger.h>
#include <zypp/Pathname.h>

namespace zypp {

  namespace media {
  
    class SuseMediaVerifier: public MediaVerifierBase
    {
      public:
	SuseMediaVerifier();
	virtual ~SuseMediaVerifier();
	
	/**
	 *
	 * \throws Exception
	 */
	virtual bool isDesiredMedia(const MediaAccessRef &ref, MediaNr mediaNr);
    };
    
  }
  
}

#endif // ZYPP_MEDIA_SUSEMEDIAVERIFIER_h
