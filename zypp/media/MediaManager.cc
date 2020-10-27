/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MediaManager.cc
 *
*/
#include <map>
#include <list>
#include <iostream>
#include <typeinfo>

#include <zypp/media/MediaException.h>
#include <zypp/media/MediaManager.h>
#include <zypp/media/MediaHandlerFactory.h>
#include <zypp/media/MediaHandler.h>
#include <zypp/media/Mount.h>

#include <zypp/base/String.h>
#include <zypp/base/Logger.h>
#include <zypp/Pathname.h>
#include <zypp/PathInfo.h>

//////////////////////////////////////////////////////////////////////
namespace zypp
{ ////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////
  namespace media
  { //////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////
    namespace // anonymous
    { ////////////////////////////////////////////////////////////////

      struct ManagedMedia;
      std::ostream & operator<<( std::ostream & str, const ManagedMedia & obj );

      // -------------------------------------------------------------
      struct ManagedMedia
      {
        ~ManagedMedia()
        {
          try
          {
            close(); // !!! make sure handler gets properly deleted.
          }
          catch(...) {}
        }

        ManagedMedia( ManagedMedia &&m )
          : desired ( m.desired )
          , verifier( std::move(m.verifier) )
          , _handler ( std::move(m._handler) )
        {}

        static ManagedMedia makeManagedMedia ( const Url &o_url, const Pathname &preferred_attach_point, const MediaVerifierRef &v )
        {
          auto handler = MediaHandlerFactory::createHandler( o_url, preferred_attach_point );
          if ( !handler ) {
            ERR << "Failed to create media handler" << std::endl;
            ZYPP_THROW( MediaSystemException(o_url, "Failed to create media handler"));
          }
          return ManagedMedia( std::move(handler), v );
        }

        ManagedMedia &operator= ( ManagedMedia &&other ) = default;

        operator bool () const {
          return ( _handler ? true : false );
        }

        inline MediaHandler &handler() {
          if ( !_handler )
              ZYPP_THROW(MediaNotOpenException("Accessing ManagedMedia after it was closed"));
          return *_handler;
        }

        inline const MediaHandler &handler() const {
          if ( !_handler )
            ZYPP_THROW(MediaNotOpenException("Accessing ManagedMedia after it was closed"));
          return *_handler;
        }

        std::ostream & dumpOn( std::ostream & str ) const {
          if ( !_handler )
            return str << "ManagedMedia( closed )";

          str << _handler->protocol() << "(" << *_handler << ")";
          return str;
        }

        inline void close ()
        {
          ///////////////////////////////////////////////////////////////////
          // !!! make shure handler gets properly deleted.
          // I.e. release attached media before deleting the handler.
          ///////////////////////////////////////////////////////////////////

          try {
            handler().release();
          }
          catch (const MediaException & excpt_r)
          {
            ZYPP_CAUGHT(excpt_r);
            WAR << "Close: " << *this << " (" << excpt_r << ")" << std::endl;
            ZYPP_RETHROW(excpt_r);
          }
          MIL << "Close: " << *this << " (OK)" << std::endl;
        }

        inline void
        checkAttached(MediaAccessId id)
        {
          if( !handler().isAttached())
          {
            DBG << "checkAttached(" << id << ") not attached" << std::endl;
            desired = false;
            ZYPP_THROW(MediaNotAttachedException(
              handler().url()
            ));
          }
        }

        inline void checkDesired( MediaAccessId id )
        {
          checkAttached( id );

          if ( !desired )
          {
            const auto &hdl = handler();
            try {
              desired = verifier->isDesiredMedia( handler() );
            } catch ( const zypp::Exception &e ) {
                ZYPP_CAUGHT( e );

                media::MediaNotDesiredException newEx ( hdl.url() );
                newEx.remember( e );
                ZYPP_THROW( newEx );
            }

            if( !desired )
            {
              DBG << "checkDesired(" << id << "): not desired (report by " << verifier->info() << ")" << std::endl;
              ZYPP_THROW( MediaNotDesiredException( hdl.url() ) );
            }

            DBG << "checkDesired(" << id << "): desired (report by " << verifier->info() << ")" << std::endl;
          } else {
            DBG << "checkDesired(" << id << "): desired (cached)" << std::endl;
          }
        }

        bool             desired;
        MediaVerifierRef verifier;
        Pathname         deltafile;

      private:
        ManagedMedia( std::unique_ptr<MediaHandler> &&h, const MediaVerifierRef &v)
          : desired (false)
          , verifier(v)
          , _handler ( std::move(h) )
        {}

        std::unique_ptr<MediaHandler>  _handler;
      };

      std::ostream & operator<<( std::ostream & str, const ManagedMedia & obj ) {
        return obj.dumpOn( str );
      }

      // -------------------------------------------------------------
      typedef std::map<MediaAccessId, ManagedMedia> ManagedMediaMap;

      ////////////////////////////////////////////////////////////////
    } // anonymous
    //////////////////////////////////////////////////////////////////


    //////////////////////////////////////////////////////////////////
    std::string
    MediaVerifierBase::info() const
    {
      return std::string(typeid((*this)).name());
    }


    //////////////////////////////////////////////////////////////////
    std::string
    NoVerifier::info() const
    {
      return std::string("zypp::media::NoVerifier");
    }


    //////////////////////////////////////////////////////////////////
    class MediaManager_Impl
    {
    private:
      friend class MediaManager;

      MediaAccessId       last_accessid;
      ManagedMediaMap     mediaMap;

      MediaManager_Impl()
        : last_accessid(0)
      {}

    public:
      ~MediaManager_Impl()
      {
        try
        {
          // remove depending (iso) handlers first
          ManagedMediaMap::iterator it;
          bool found;
          do
          {
            found = false;
            for(it = mediaMap.begin(); it != mediaMap.end(); /**/)
            {
              if( it->second && it->second.handler().dependsOnParent() )
              {
                found = true;
                // let it forget its parent, we will
                // destroy it later (in clear())...
                it->second.handler().resetParentId();
                it = mediaMap.erase( it ); // postfix! Incrementing before erase
              } else {
                ++it;
              }
            }
          } while(found);

          // remove all other handlers
          mediaMap.clear();
        }
        catch( ... )
        {}
      }

      inline MediaAccessId
      nextAccessId()
      {
        return ++last_accessid;
      }

      inline bool
      hasId(MediaAccessId accessId) const
      {
        return mediaMap.find(accessId) != mediaMap.end();
      }

      inline ManagedMedia &
      findMM(MediaAccessId accessId)
      {
        ManagedMediaMap::iterator it( mediaMap.find(accessId));
        if( it == mediaMap.end())
        {
          ZYPP_THROW(MediaNotOpenException(
            "Invalid media access id " + str::numstring(accessId)
          ));
        }
        return it->second;
      }

      static inline time_t
      getMountTableMTime()
      {
        time_t mtime = zypp::PathInfo("/etc/mtab").mtime();
        if( mtime <= 0)
        {
          WAR << "Failed to retrieve modification time of '/etc/mtab'"
              << std::endl;
        }
        return mtime;
      }

      static inline MountEntries
      getMountEntries()
      {
        return Mount::getEntries();
      }

    };


    //////////////////////////////////////////////////////////////////
    // STATIC
    zypp::RW_pointer<MediaManager_Impl> MediaManager::m_impl;


    //////////////////////////////////////////////////////////////////
    MediaManager::MediaManager()
    {
      if( !m_impl)
      {
        m_impl.reset( new MediaManager_Impl());
      }
    }

    // ---------------------------------------------------------------
    MediaManager::~MediaManager()
    {
    }

    // ---------------------------------------------------------------
    MediaAccessId
    MediaManager::open(const Url &url, const Pathname &preferred_attach_point)
    {
      // create new access handler for it
      MediaVerifierRef verifier( new NoVerifier());
      ManagedMedia tmp = ManagedMedia::makeManagedMedia( url, preferred_attach_point, verifier );

      MediaAccessId nextId = m_impl->nextAccessId();

      m_impl->mediaMap.insert( std::make_pair( nextId, std::move(tmp) ) );
      //m_impl->mediaMap[nextId] = std::move(tmp);

      DBG << "Opened new media access using id " << nextId
          << " to " << url.asString() << std::endl;
      return nextId;
    }

    // ---------------------------------------------------------------
    void
    MediaManager::close(MediaAccessId accessId)
    {
      //
      // The MediaISO handler internally requests an accessId
      // of a "parent" handler providing the iso file.
      // The parent handler accessId is private to MediaISO,
      // but the attached media source may be shared reference.
      // This means, that if the accessId exactly matches the
      // parent handler id, close was used on uninitialized
      // accessId variable (or the accessId was guessed) and
      // the close request to this id will be rejected here.
      //
      ManagedMediaMap::iterator m(m_impl->mediaMap.begin());
      for( ; m != m_impl->mediaMap.end(); ++m)
      {
        if( m->second.handler().dependsOnParent(accessId, true))
        {
          ZYPP_THROW(MediaIsSharedException(
            m->second.handler().url().asString()
          ));
        }
      }

      DBG << "Close to access handler using id "
          << accessId << " requested" << std::endl;

      ManagedMedia &ref( m_impl->findMM(accessId));
      ref.close();

      m_impl->mediaMap.erase(accessId);
    }

    // ---------------------------------------------------------------
    bool
    MediaManager::isOpen(MediaAccessId accessId) const
    {
      ManagedMediaMap::iterator it( m_impl->mediaMap.find(accessId));
      return it != m_impl->mediaMap.end();
    }

    // ---------------------------------------------------------------
    std::string
    MediaManager::protocol(MediaAccessId accessId) const
    {
      ManagedMedia &ref( m_impl->findMM(accessId));

      return ref.handler().protocol();
    }

    // ---------------------------------------------------------------
	  bool
    MediaManager::downloads(MediaAccessId accessId) const
    {
      ManagedMedia &ref( m_impl->findMM(accessId));

      return ref.handler().downloads();
    }

    // ---------------------------------------------------------------
    Url
    MediaManager::url(MediaAccessId accessId) const
    {
      ManagedMedia &ref( m_impl->findMM(accessId));

      return ref.handler().url();
    }

    // ---------------------------------------------------------------
    void
    MediaManager::addVerifier(MediaAccessId           accessId,
                              const MediaVerifierRef &verifier)
    {
      if( !verifier)
        ZYPP_THROW(MediaException("Invalid verifier reference"));

      ManagedMedia &ref( m_impl->findMM(accessId));

      ref.desired = false;
      MediaVerifierRef(verifier).swap(ref.verifier);

      DBG << "MediaVerifier change: id=" << accessId << ", verifier="
          << verifier->info() << std::endl;
    }

    // ---------------------------------------------------------------
    void
    MediaManager::delVerifier(MediaAccessId accessId)
    {
      ManagedMedia &ref( m_impl->findMM(accessId));

      MediaVerifierRef verifier( new NoVerifier());
      ref.desired  = false;
      ref.verifier.swap(verifier);

      DBG << "MediaVerifier change: id=" << accessId << ", verifier="
          << verifier->info() << std::endl;
    }

    // ---------------------------------------------------------------
    bool
    MediaManager::setAttachPrefix(const Pathname &attach_prefix)
    {
      return MediaHandler::setAttachPrefix(attach_prefix);
    }

    // ---------------------------------------------------------------
    void MediaManager::attach(MediaAccessId accessId)
    {
      ManagedMedia &ref( m_impl->findMM(accessId));
      auto &hdl = ref.handler();

      DBG << "attach(id=" << accessId << ")" << std::endl;

      // try first mountable/mounted device
      hdl.attach(false);
      try
      {
        ref.checkDesired(accessId);
        return;
      }
      catch (const MediaException & ex)
      {
        ZYPP_CAUGHT(ex);

        if (!hdl.hasMoreDevices())
          ZYPP_RETHROW(ex);

        if (hdl.isAttached())
          hdl.release();
      }

      MIL << "checkDesired(" << accessId << ") of first device failed,"
        " going to try others with attach(true)" << std::endl;

      while (hdl.hasMoreDevices())
      {
        try
        {
          // try to attach next device
          hdl.attach(true);
          ref.checkDesired(accessId);
          return;
        }
        catch (const MediaNotDesiredException & ex)
        {
          ZYPP_CAUGHT(ex);

          if (!hdl.hasMoreDevices())
          {
            MIL << "No desired media found after trying all detected devices." << std::endl;
            ZYPP_RETHROW(ex);
          }

          AttachedMedia media(hdl.attachedMedia());
          DBG << "Skipping " << media.mediaSource->asString() << ": not desired media." << std::endl;

          hdl.release();
        }
        catch (const MediaException & ex)
        {
          ZYPP_CAUGHT(ex);

          if (!hdl.hasMoreDevices())
            ZYPP_RETHROW(ex);

          AttachedMedia media(hdl.attachedMedia());
          DBG << "Skipping " << media.mediaSource->asString() << " because of exception thrown by attach(true)" << std::endl;

          if (hdl.isAttached()) hdl.release();
        }
      }
    }

    // ---------------------------------------------------------------
    void
    MediaManager::release(MediaAccessId accessId, const std::string & ejectDev)
    {
      ManagedMedia &ref( m_impl->findMM(accessId));

      DBG << "release(id=" << accessId;
      if (!ejectDev.empty())
        DBG << ", " << ejectDev;
      DBG << ")" << std::endl;

      if(!ejectDev.empty())
      {
        //
        // release MediaISO handlers, that are using the one
        // specified with accessId, because it provides the
        // iso file and it will disappear now (forced release
        // with eject).
        //
        ManagedMediaMap::iterator m(m_impl->mediaMap.begin());
        for( ; m != m_impl->mediaMap.end(); ++m)
        {
          auto &hdl = m->second.handler();
          if( hdl.dependsOnParent(accessId, false))
          {
            try
            {
              DBG << "Forcing release of handler depending on access id "
                  << accessId << std::endl;
              m->second.desired  = false;
              hdl.release();
            }
            catch(const MediaException &e)
            {
              ZYPP_CAUGHT(e);
            }
          }
        }
      }
      ref.desired  = false;
      ref.handler().release(ejectDev);
    }

    // ---------------------------------------------------------------
    void
    MediaManager::releaseAll()
    {
      MIL << "Releasing all attached media" << std::endl;

      ManagedMediaMap::iterator m(m_impl->mediaMap.begin());
      for( ; m != m_impl->mediaMap.end(); ++m)
      {
        auto &hdl = m->second.handler();
        if( hdl.dependsOnParent())
          continue;

        try
        {
          if(hdl.isAttached())
          {
            DBG << "Releasing media id " << m->first << std::endl;
            m->second.desired  = false;
            hdl.release();
          }
          else
          {
            DBG << "Media id " << m->first << " not attached " << std::endl;
          }
        }
        catch(const MediaException & e)
        {
          ZYPP_CAUGHT(e);
          ERR << "Failed to release media id " << m->first << std::endl;
        }
      }

      MIL << "Exit" << std::endl;
    }

    // ---------------------------------------------------------------
    void
    MediaManager::disconnect(MediaAccessId accessId)
    {
      ManagedMedia &ref( m_impl->findMM(accessId));

      ref.handler().disconnect();
    }

    // ---------------------------------------------------------------
    bool
    MediaManager::isAttached(MediaAccessId accessId) const
    {
      ManagedMedia &ref( m_impl->findMM(accessId));

      return ref.handler().isAttached();
    }

    // ---------------------------------------------------------------
    bool MediaManager::isSharedMedia(MediaAccessId accessId) const
    {
      ManagedMedia &ref( m_impl->findMM(accessId));

      return ref.handler().isSharedMedia();
    }

    // ---------------------------------------------------------------
    bool
    MediaManager::isDesiredMedia(MediaAccessId accessId) const
    {
      ManagedMedia &ref( m_impl->findMM(accessId));

      if( !ref.handler().isAttached())
      {
        ref.desired = false;
      }
      else
      {
        try {
          ref.desired = ref.verifier->isDesiredMedia( ref.handler() );
        }
        catch(const zypp::Exception &e) {
          ZYPP_CAUGHT(e);
          ref.desired = false;
        }
      }
      DBG << "isDesiredMedia(" << accessId << "): "
          << (ref.desired ? "" : "not ")
          << "desired (report by "
          << ref.verifier->info() << ")" << std::endl;
      return ref.desired;
    }

    // ---------------------------------------------------------------
    bool
    MediaManager::isDesiredMedia(MediaAccessId           accessId,
                                 const MediaVerifierRef &verifier) const
    {
      MediaVerifierRef v(verifier);
      if( !v)
        ZYPP_THROW(MediaException("Invalid verifier reference"));

      ManagedMedia &ref( m_impl->findMM(accessId));

      bool desired = false;
      if( ref.handler().isAttached())
      {
        try {
          desired = v->isDesiredMedia( ref.handler() );
        }
        catch(const zypp::Exception &e) {
          ZYPP_CAUGHT(e);
          desired = false;
        }
      }
      DBG << "isDesiredMedia(" << accessId << "): "
          << (desired ? "" : "not ")
          << "desired (report by "
          << v->info() << ")" << std::endl;
      return desired;
    }

    // ---------------------------------------------------------------
    bool
    MediaManager::isChangeable(MediaAccessId accessId)
    {
      return url(accessId).getScheme() == "cd" || url(accessId).getScheme() == "dvd";
    }

    // ---------------------------------------------------------------
    Pathname
    MediaManager::localRoot(MediaAccessId accessId) const
    {
      ManagedMedia &ref( m_impl->findMM(accessId));

      Pathname path;
      path = ref.handler().localRoot();
      return path;
    }

    // ---------------------------------------------------------------
    Pathname
    MediaManager::localPath(MediaAccessId accessId,
                            const Pathname & pathname) const
    {
      ManagedMedia &ref( m_impl->findMM(accessId));

      Pathname path;
      path = ref.handler().localPath(pathname);
      return path;
    }

    void
    MediaManager::provideFile(MediaAccessId   accessId,
                              const Pathname &filename,
                              const ByteCount &expectedFileSize ) const
    {
      ManagedMedia &ref( m_impl->findMM(accessId));

      auto loc = OnMediaLocation( filename )
                   .setDownloadSize( expectedFileSize )
                   .setDeltafile( ref.deltafile );

      provideFile( accessId, loc );
    }

    // ---------------------------------------------------------------
    void
    MediaManager::provideFile(MediaAccessId   accessId,
                              const Pathname &filename ) const
    {
      ManagedMedia &ref( m_impl->findMM(accessId));

      auto loc = OnMediaLocation( filename )
                   .setDeltafile( ref.deltafile );

      provideFile( accessId, loc );
    }

    void MediaManager::provideFile( MediaAccessId accessId, const OnMediaLocation &file ) const
    {
      ManagedMedia &ref( m_impl->findMM(accessId));

      ref.checkDesired(accessId);

      ref.handler().provideFile( file );
    }

    // ---------------------------------------------------------------
    void
    MediaManager::setDeltafile(MediaAccessId   accessId,
                              const Pathname &filename ) const
    {
      ManagedMedia &ref( m_impl->findMM(accessId));

      ref.checkDesired(accessId);

      ref.deltafile = filename;
    }

    void MediaManager::precacheFiles(MediaAccessId accessId, const std::vector<OnMediaLocation> &files)
    {
      ManagedMedia &ref( m_impl->findMM(accessId));

      ref.checkDesired(accessId);

      ref.handler().precacheFiles( files );
    }

    // ---------------------------------------------------------------
    void
    MediaManager::provideDir(MediaAccessId   accessId,
                             const Pathname &dirname) const
    {
      ManagedMedia &ref( m_impl->findMM(accessId));

      ref.checkDesired(accessId);

      ref.handler().provideDir(dirname);
    }

    // ---------------------------------------------------------------
    void
    MediaManager::provideDirTree(MediaAccessId   accessId,
                                 const Pathname &dirname) const
    {
      ManagedMedia &ref( m_impl->findMM(accessId));

      ref.checkDesired(accessId);

      ref.handler().provideDirTree(dirname);
    }

    // ---------------------------------------------------------------
    void
    MediaManager::releaseFile(MediaAccessId   accessId,
                              const Pathname &filename) const
    {
      ManagedMedia &ref( m_impl->findMM(accessId));

      ref.checkAttached(accessId);

      ref.handler().releaseFile(filename);
    }

    // ---------------------------------------------------------------
    void
    MediaManager::releaseDir(MediaAccessId   accessId,
                             const Pathname &dirname) const
    {
      ManagedMedia &ref( m_impl->findMM(accessId));

      ref.checkAttached(accessId);

      ref.handler().releaseDir(dirname);
    }


    // ---------------------------------------------------------------
    void
    MediaManager::releasePath(MediaAccessId   accessId,
                              const Pathname &pathname) const
    {
      ManagedMedia &ref( m_impl->findMM(accessId));

      ref.checkAttached(accessId);

      ref.handler().releasePath(pathname);
    }

    // ---------------------------------------------------------------
    void
    MediaManager::dirInfo(MediaAccessId           accessId,
                          std::list<std::string> &retlist,
                          const Pathname         &dirname,
                          bool                    dots) const
    {
      ManagedMedia &ref( m_impl->findMM(accessId));

      // FIXME: ref.checkDesired(accessId); ???
      ref.checkAttached(accessId);

      ref.handler().dirInfo(retlist, dirname, dots);
    }

    // ---------------------------------------------------------------
    void
    MediaManager::dirInfo(MediaAccessId           accessId,
                          filesystem::DirContent &retlist,
                          const Pathname         &dirname,
                          bool                    dots) const
    {
      ManagedMedia &ref( m_impl->findMM(accessId));

      // FIXME: ref.checkDesired(accessId); ???
      ref.checkAttached(accessId);

      ref.handler().dirInfo(retlist, dirname, dots);
    }

    // ---------------------------------------------------------------
    bool
    MediaManager::doesFileExist(MediaAccessId  accessId, const Pathname & filename ) const
    {
      ManagedMedia &ref( m_impl->findMM(accessId));

      // FIXME: ref.checkDesired(accessId); ???
      ref.checkAttached(accessId);

      return ref.handler().doesFileExist(filename);
    }

    // ---------------------------------------------------------------
    void
    MediaManager::getDetectedDevices(MediaAccessId accessId,
                                     std::vector<std::string> & devices,
                                     unsigned int & index) const
    {
      ManagedMedia &ref( m_impl->findMM(accessId));
      return ref.handler().getDetectedDevices(devices, index);
    }

    // ---------------------------------------------------------------
    // STATIC
    time_t
    MediaManager::getMountTableMTime()
    {
      return MediaManager_Impl::getMountTableMTime();
    }

    // ---------------------------------------------------------------
    // STATIC
    MountEntries
    MediaManager::getMountEntries()
    {
      return MediaManager_Impl::getMountEntries();
    }

    // ---------------------------------------------------------------
    bool
    MediaManager::isUseableAttachPoint(const Pathname &path,
                                       bool            mtab) const
    {
      if( path.empty() || path == "/" || !PathInfo(path).isDir())
        return false;

      //
      // check against our current attach points
      //
      ManagedMediaMap::const_iterator m(m_impl->mediaMap.begin());
      for( ; m != m_impl->mediaMap.end(); ++m)
      {
        AttachedMedia ret = m->second.handler().attachedMedia();
        if( ret.mediaSource && ret.attachPoint)
        {
          std::string mnt(ret.attachPoint->path.asString());
          std::string our(path.asString());

          if( our == mnt)
          {
            // already used as attach point
            return false;
          }
          else
          if( mnt.size() > our.size()   &&
              mnt.at(our.size()) == '/' &&
             !mnt.compare(0, our.size(), our))
          {
            // mountpoint is bellow of path
            // (would hide the content)
            return false;
          }
        }
      }

      if( !mtab)
        return true;

      //
      // check against system mount entries
      //
      MountEntries  entries( m_impl->getMountEntries());
      MountEntries::const_iterator e;
      for( e = entries.begin(); e != entries.end(); ++e)
      {
        std::string mnt(Pathname(e->dir).asString());
        std::string our(path.asString());

        if( our == mnt)
        {
          // already used as mountpoint
          return false;
        }
        else
        if( mnt.size() > our.size()   &&
            mnt.at(our.size()) == '/' &&
           !mnt.compare(0, our.size(), our))
        {
          // mountpoint is bellow of path
          // (would hide the content)
          return false;
        }
      }

      return true;
    }

    // ---------------------------------------------------------------
    AttachedMedia
    MediaManager::getAttachedMedia(MediaAccessId &accessId) const
    {
      ManagedMedia &ref( m_impl->findMM(accessId));

      return ref.handler().attachedMedia();
    }

    // ---------------------------------------------------------------
    AttachedMedia
    MediaManager::findAttachedMedia(const MediaSourceRef &media) const
    {
      if( !media || media->type.empty())
        return AttachedMedia();

      ManagedMediaMap::const_iterator m(m_impl->mediaMap.begin());
      for( ; m != m_impl->mediaMap.end(); ++m)
      {
        if( !m->second.handler().isAttached())
          continue;

        AttachedMedia ret = m->second.handler().attachedMedia();
        if( ret.mediaSource && ret.mediaSource->equals( *media))
            return ret;
      }
      return AttachedMedia();
    }

    // ---------------------------------------------------------------
    void
    MediaManager::forceReleaseShared(const MediaSourceRef &media)
    {
      if( !media || media->type.empty())
        return;

      ManagedMediaMap::iterator m(m_impl->mediaMap.begin());
      for( ; m != m_impl->mediaMap.end(); ++m)
      {
        if( !m->second.handler().isAttached())
          continue;

        AttachedMedia ret = m->second.handler().attachedMedia();
        if( ret.mediaSource && ret.mediaSource->equals( *media))
        {
          m->second.handler().release();
          m->second.desired  = false;
        }
      }
    }

    //////////////////////////////////////////////////////////////////
  } // namespace media
  ////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////
} // namespace zypp
//////////////////////////////////////////////////////////////////////
/*
** vim: set ts=2 sts=2 sw=2 ai et:
*/
