/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MediaISO.h
 *
 */
#ifndef ZYPP_MEDIA_MEDIAISO_H
#define ZYPP_MEDIA_MEDIAISO_H

#include "zypp/media/MediaHandler.h"
#include "zypp/media/MediaManager.h"

//////////////////////////////////////////////////////////////////////
namespace zypp
{ ////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////
  namespace media
  { //////////////////////////////////////////////////////////////////


    ///////////////////////////////////////////////////////////////////
    //
    // CLASS NAME : MediaISO
    //
    /**
     * @short Implementation class for ISO MediaHandler
     * @see MediaHandler
     **/
    class MediaISO : public MediaHandler
    {
      private:
        Pathname      _isofile;
        MediaAccessId _isosource;
        std::string   _filesystem;

      protected:

        MEDIA_HANDLER_API;

      public:

        MediaISO(const Url      &url_r,
                 const Pathname &attach_point_hint_r);

        virtual
        ~MediaISO();

        virtual bool
        isAttached() const;
/*
	      virtual bool
        attachesMediaSource(const MediaSourceRef &media) const;
*/
    };


    //////////////////////////////////////////////////////////////////
  } // namespace media
  ////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////
} // namespace zypp
//////////////////////////////////////////////////////////////////////

#endif // ZYPP_MEDIA_MEDIAISO_H

// vim: set ts=2 sts=2 sw=2 ai et:

