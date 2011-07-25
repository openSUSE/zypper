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

#include <iosfwd>

#include "zypp/Pathname.h"
#include "zypp/base/String.h"
#include "zypp/base/PtrTypes.h"


namespace zypp {
  namespace media {

    ///////////////////////////////////////////////////////////////////
    /**
     * Media manager access Id type.
     */
    typedef unsigned int MediaAccessId;


    ///////////////////////////////////////////////////////////////////
    /**
     * Media source internally used by MediaManager and MediaHandler.
     */
    class MediaSource
    {
    public:
      MediaSource(const std::string &_type,  const std::string &_name,
                  unsigned int       _maj=0, unsigned int       _min=0,
		  const std::string &_bdir=std::string(), bool  _own=true)
        : maj_nr(_maj)
        , min_nr(_min)
        , type(_type)
        , name(_name)
	, bdir(_bdir)
	, iown(_own)
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
	std::string tmp1;
        if(maj_nr != 0)
	{
	  tmp1 = "[" + str::numstring(maj_nr) + "," +
	               str::numstring(min_nr) + "]";
	}
        return type + "<" + name + tmp1 + ">";
      }

      unsigned int maj_nr;  //!< A major number if source is a device.
      unsigned int min_nr;  //!< A minor number if source is a device.
      std::string  type;    //!< A media handler specific source type.
      std::string  name;    //!< A media handler specific source name.
      std::string  bdir;    //!< Directory, the media may be bound to.
      bool         iown;    //!< True, if mounted by media manager.
    };

    /** \relates MediaSource Stream output */
    inline std::ostream & operator<<( std::ostream & str, const MediaSource & obj )
    { return str << obj.asString(); }

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

    /** \relates AttachPoint Stream output */
    std::ostream & operator<<( std::ostream & str, const AttachPoint & obj );

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

    /** \relates AttachedMedia Stream output */
    std::ostream & operator<<( std::ostream & str, const AttachedMedia & obj );

  } // namespace media
} // namespace zypp


#endif // ZYPP_MEDIA_MEDIASOURCE_H

