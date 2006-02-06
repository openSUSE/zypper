/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MediaSource.h
 *
 */
#ifndef ZYPP_MEDIA_MEDIASOURCE_H
#define ZYPP_MEDIA_MEDIASOURCE_H

#include "zypp/Pathname.h"
#include "zypp/base/String.h"
#include "zypp/base/PtrTypes.h"


namespace zypp {
  namespace media {


    ///////////////////////////////////////////////////////////////////
    /**
     * Media source internally used by MediaManager and MediaHandler.
     */
    class MediaSource
    {
    public:
      MediaSource(const std::string &_type,  const std::string &_name,
                  unsigned int       _maj=0, unsigned int       _min=0)
        : maj_nr(_maj)
        , min_nr(_min)
        , type(_type)
        , name(_name)
      {}

      MediaSource()
        : maj_nr(0)
        , min_nr(0)
      {}

      virtual
      ~MediaSource()
      {}

      /**
       * Check if the both sources are equal.
       */
      virtual bool equals(const MediaSource &src) const
      {
        if( type == src.type)
        {
          if( maj_nr == 0)
            return name == src.name;
          else
            return maj_nr == src.maj_nr &&
                   min_nr == src.min_nr;
        }
        return false;
      }

      /**
       * Return media source as string for debuging purposes.
       */
      virtual std::string asString() const
      {
        return type + ":" + name      + "(" +
               str::numstring(maj_nr) + "," +
               str::numstring(min_nr) + ")";
      }

      unsigned int maj_nr;  //!< A major number if source is a device.
      unsigned int min_nr;  //!< A minor number if source is a device.
      std::string  type;    //!< A media handler specific source type.
      std::string  name;    //!< A media handler specific source name.
    };


    ///////////////////////////////////////////////////////////////////
    /**
     * Attach point of a media source.
     */
    class AttachPoint
    {
    public:
      AttachPoint(const Pathname &_path=Pathname(),
                  bool            _temp=true)
        : path(_path)
        , temp(_temp)
      {}

      bool empty() const { return path.empty(); }

      Pathname path;	//!< The path name (mount point).
      bool     temp;    //!< If it was created temporary.
    };


    ///////////////////////////////////////////////////////////////////
    typedef zypp::RW_pointer<MediaSource> MediaSourceRef;
    typedef zypp::RW_pointer<AttachPoint> AttachPointRef;


    ///////////////////////////////////////////////////////////////////
    /**
     * A simple structure containing references
     * to a media source and its attach point.
     */
    struct AttachedMedia
    {
      AttachedMedia()
      {}

      AttachedMedia(const MediaSourceRef &_mediaSource,
                    const AttachPointRef &_attachPoint)
     	: mediaSource( _mediaSource)
	, attachPoint( _attachPoint)
      {}

      MediaSourceRef mediaSource;
      AttachPointRef attachPoint;
    };


  } // namespace media
} // namespace zypp


#endif // ZYPP_MEDIA_MEDIASOURCE_H

