/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/SilentCallbacks.h
 *
*/
#ifndef ZYPP_SILENTCALLBACKS_H
#define ZYPP_SILENTCALLBACKS_H

#include "zypp/ZYppCallbacks.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  namespace media 
  { 
    // media change request callback
    struct SilentMediaChange : public callback::ReceiveReport<MediaChangeReport>
    {
      virtual Action requestMedia(
        const Source_Ref /*source*/
	, unsigned 
	, Error 
	, std::string
      ) { return MediaChangeReport::ABORT; }
    };

    /////////////////////////////////////////////////////////////////
  } // namespace media
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

#endif // ZYPP_SILENTCALLBACKS_H
