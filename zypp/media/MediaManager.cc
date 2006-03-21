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
#include <zypp/media/MediaException.h>
#include <zypp/media/MediaManager.h>
#include <zypp/media/MediaHandler.h>
#include <zypp/media/Mount.h>
#include <zypp/thread/Mutex.h>
#include <zypp/thread/MutexLock.h>

#include <zypp/base/String.h>
#include <zypp/base/Logger.h>
#include <zypp/Pathname.h>
#include <zypp/PathInfo.h>

#include <map>
#include <list>
#include <iostream>


//////////////////////////////////////////////////////////////////////
namespace zypp
{ ////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////
  namespace media
  { //////////////////////////////////////////////////////////////////

    using zypp::thread::Mutex;
    using zypp::thread::MutexLock;

    //////////////////////////////////////////////////////////////////
    namespace // anonymous
    { ////////////////////////////////////////////////////////////////


      // -------------------------------------------------------------
      // STATIC
      static Mutex  g_Mutex;


      // -------------------------------------------------------------
      struct ManagedMedia
      {
        ~ManagedMedia()
        {}

        ManagedMedia()
          : desired (false)
        {}

        ManagedMedia(const ManagedMedia &m)
          : desired (m.desired)
          , handler (m.handler)
          , verifier(m.verifier)
        {}

        ManagedMedia(const MediaAccessRef &h, const MediaVerifierRef &v)
          : desired (false)
          , handler (h)
          , verifier(v)
        {}

        inline void
        checkAttached(MediaAccessId id)
        {
          if( !handler->isAttached())
          {
            DBG << "checkAttached(" << id << ") not attached" << std::endl;
            desired = false;
            ZYPP_THROW(MediaNotAttachedException(
              handler->url()
            ));
          }
        }

        inline void
        checkDesired(MediaAccessId id)
        {
          checkAttached(id);

          if( !desired)
          {
            try {
              desired = verifier->isDesiredMedia(handler);
            }
            catch(const zypp::Exception &e) {
              ZYPP_CAUGHT(e);
              desired = false;
            }

            if( !desired)
            {
              DBG << "checkDesired(" << id << "): not desired" << std::endl;
              ZYPP_THROW(MediaNotDesiredException(
                handler->url()
              ));
            }

            DBG << "checkDesired(" << id << "): desired (report)" << std::endl;
          } else {
            DBG << "checkDesired(" << id << "): desired (cached)" << std::endl;
          }
        }

        bool             desired;
        MediaAccessRef   handler;
        MediaVerifierRef verifier;
      };


      // -------------------------------------------------------------
      typedef std::map<MediaAccessId, ManagedMedia> ManagedMediaMap;


      ////////////////////////////////////////////////////////////////
    } // anonymous
    //////////////////////////////////////////////////////////////////


    //////////////////////////////////////////////////////////////////
    class MediaManager_Impl
    {
    private:
      friend class MediaManager;

      MediaAccessId   last_accessid;
      ManagedMediaMap mediaMap;

      MediaManager_Impl()
        : last_accessid(0)
      {}

    public:
      ~MediaManager_Impl()
      {
        MutexLock glock(g_Mutex);

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
              if( it->second.handler->dependsOnParent())
              {
                found = true;
                // let it forget its parent, we will
                // destroy it later (in clear())...
                it->second.handler->resetParentId();
                mediaMap.erase( it++ ); // postfix! Incrementing before erase
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
        return zypp::PathInfo("/etc/mtab").mtime();
      }

      static inline MountEntries
      getMountEntries()
      {
        return Mount::getEntries("/etc/mtab");
      }

    };


    //////////////////////////////////////////////////////////////////
    // STATIC
    zypp::RW_pointer<MediaManager_Impl> MediaManager::m_impl(NULL);


    //////////////////////////////////////////////////////////////////
    MediaManager::MediaManager()
    {
      MutexLock glock(g_Mutex);
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
      MutexLock glock(g_Mutex);

      // create new access handler for it
      MediaAccessRef handler( new MediaAccess());
      MediaVerifierRef verifier( new NoVerifier());
      ManagedMedia tmp( handler, verifier);

      tmp.handler->open(url, preferred_attach_point);

      MediaAccessId nextId = m_impl->nextAccessId();

      m_impl->mediaMap[nextId] = tmp;

      DBG << "Opened new media access using id " << nextId
          << " to " << url.asString() << std::endl;
      return nextId;
    }

    // ---------------------------------------------------------------
    void
    MediaManager::close(MediaAccessId accessId)
    {
      MutexLock glock(g_Mutex);

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
        if( m->second.handler->dependsOnParent(accessId, true))
        {
          ZYPP_THROW(MediaIsSharedException(
            m->second.handler->url().asString()
          ));
        }
      }

      DBG << "Close to access handler using id "
          << accessId << " requested" << std::endl;

      ManagedMedia &ref( m_impl->findMM(accessId));
      ref.handler->close();

      m_impl->mediaMap.erase(accessId);
    }

    // ---------------------------------------------------------------
    bool
    MediaManager::isOpen(MediaAccessId accessId) const
    {
      MutexLock glock(g_Mutex);

      ManagedMediaMap::iterator it( m_impl->mediaMap.find(accessId));
      return it != m_impl->mediaMap.end() &&
             it->second.handler->isOpen();
    }

    // ---------------------------------------------------------------
    std::string
    MediaManager::protocol(MediaAccessId accessId) const
    {
      MutexLock glock(g_Mutex);

      ManagedMedia &ref( m_impl->findMM(accessId));

      return ref.handler->protocol();
    }

    // ---------------------------------------------------------------
	  bool
    MediaManager::downloads(MediaAccessId accessId) const
    {
      MutexLock glock(g_Mutex);

      ManagedMedia &ref( m_impl->findMM(accessId));

      return ref.handler->downloads();
    }

    // ---------------------------------------------------------------
    // STATIC
    bool
    MediaManager::downloads(const Url &url)
    {
      return MediaAccess::downloads( url);
    }

    // ---------------------------------------------------------------
    Url
    MediaManager::url(MediaAccessId accessId) const
    {
      MutexLock glock(g_Mutex);

      ManagedMedia &ref( m_impl->findMM(accessId));

      return ref.handler->url();
    }

    // ---------------------------------------------------------------
    void
    MediaManager::addVerifier(MediaAccessId           accessId,
                              const MediaVerifierRef &verifier)
    {
      MutexLock glock(g_Mutex);

      if( !verifier)
        ZYPP_THROW(MediaException("Invalid verifier reference"));

      ManagedMedia &ref( m_impl->findMM(accessId));

      MediaVerifierRef(verifier).swap(ref.verifier);
      ref.desired = false;
    }

    // ---------------------------------------------------------------
    void
    MediaManager::delVerifier(MediaAccessId accessId)
    {
      MutexLock glock(g_Mutex);

      ManagedMedia &ref( m_impl->findMM(accessId));

      MediaVerifierRef verifier( new NoVerifier());
      ref.verifier.swap(verifier);
      ref.desired  = false;
    }

    // ---------------------------------------------------------------
    bool
    MediaManager::setAttachPrefix(const Pathname &attach_prefix)
    {
      MutexLock glock(g_Mutex);

      return MediaHandler::setAttachPrefix(attach_prefix);
    }

    // ---------------------------------------------------------------
    void
    MediaManager::attach(MediaAccessId accessId, bool next)
    {
      MutexLock glock(g_Mutex);

      ManagedMedia &ref( m_impl->findMM(accessId));

      DBG << "attach(id=" << accessId << ")" << std::endl;
      return ref.handler->attach(next);
    }

    // ---------------------------------------------------------------
    void
    MediaManager::release(MediaAccessId accessId, bool eject)
    {
      MutexLock glock(g_Mutex);

      ManagedMedia &ref( m_impl->findMM(accessId));

      DBG << "release(id=" << accessId
          << (eject ? ", eject)" : ")") << std::endl;
      if( eject)
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
          if( m->second.handler->dependsOnParent(accessId, false))
          {
            try
            {
              DBG << "Forcing release of handler depending on access id "
                  << accessId << std::endl;
              m->second.handler->release(!eject);
              m->second.desired  = false;
            }
            catch(const MediaException &e)
            {
              ZYPP_CAUGHT(e);
            }
          }
        }
      }
      ref.handler->release(eject);
      ref.desired  = false;
    }

    // ---------------------------------------------------------------
    void
    MediaManager::disconnect(MediaAccessId accessId)
    {
      MutexLock glock(g_Mutex);

      ManagedMedia &ref( m_impl->findMM(accessId));

      ref.handler->disconnect();
    }

    // ---------------------------------------------------------------
    bool
    MediaManager::isAttached(MediaAccessId accessId) const
    {
      MutexLock glock(g_Mutex);

      ManagedMedia &ref( m_impl->findMM(accessId));

      return ref.handler->isAttached();
    }

    // ---------------------------------------------------------------
    bool MediaManager::isSharedMedia(MediaAccessId accessId) const
    {
      MutexLock glock(g_Mutex);

      ManagedMedia &ref( m_impl->findMM(accessId));

      return ref.handler->isSharedMedia();
    }

    // ---------------------------------------------------------------
    bool
    MediaManager::isDesiredMedia(MediaAccessId accessId) const
    {
      MutexLock glock(g_Mutex);

      ManagedMedia &ref( m_impl->findMM(accessId));

      if( !ref.handler->isAttached())
      {
        ref.desired = false;
      }
      else
      {
        try {
          ref.desired = ref.verifier->isDesiredMedia(ref.handler);
        }
        catch(const zypp::Exception &e) {
          ZYPP_CAUGHT(e);
          ref.desired = false;
        }
      }
      return ref.desired;
    }

    // ---------------------------------------------------------------
    bool
    MediaManager::isDesiredMedia(MediaAccessId           accessId,
                                 const MediaVerifierRef &verifier) const
    {
      MutexLock glock(g_Mutex);

      MediaVerifierRef v(verifier);
      if( !v)
        ZYPP_THROW(MediaException("Invalid verifier reference"));

      ManagedMedia &ref( m_impl->findMM(accessId));

      bool desired = false;
      if( ref.handler->isAttached())
      {
        try {
          desired = v->isDesiredMedia(ref.handler);
        }
        catch(const zypp::Exception &e) {
          ZYPP_CAUGHT(e);
          desired = false;
        }
      }
      return desired;
    }

    // ---------------------------------------------------------------
    Pathname
    MediaManager::localRoot(MediaAccessId accessId) const
    {
      MutexLock glock(g_Mutex);

      ManagedMedia &ref( m_impl->findMM(accessId));

      Pathname path;
      path = ref.handler->localRoot();
      return path;
    }

    // ---------------------------------------------------------------
    Pathname
    MediaManager::localPath(MediaAccessId accessId,
                            const Pathname & pathname) const
    {
      MutexLock glock(g_Mutex);

      ManagedMedia &ref( m_impl->findMM(accessId));

      Pathname path;
      path = ref.handler->localPath(pathname);
      return path;
    }

    // ---------------------------------------------------------------
    void
    MediaManager::provideFile(MediaAccessId   accessId,
                              const Pathname &filename,
                              bool            cached,
                              bool            checkonly) const
    {
      MutexLock glock(g_Mutex);

      ManagedMedia &ref( m_impl->findMM(accessId));

      ref.checkDesired(accessId);

      ref.handler->provideFile(filename, cached, checkonly);
    }

    // ---------------------------------------------------------------
    void
    MediaManager::provideDir(MediaAccessId   accessId,
                             const Pathname &dirname) const
    {
      MutexLock glock(g_Mutex);

      ManagedMedia &ref( m_impl->findMM(accessId));

      ref.checkDesired(accessId);

      ref.handler->provideDir(dirname);
    }

    // ---------------------------------------------------------------
    void
    MediaManager::provideDirTree(MediaAccessId   accessId,
                                 const Pathname &dirname) const
    {
      MutexLock glock(g_Mutex);

      ManagedMedia &ref( m_impl->findMM(accessId));

      ref.checkDesired(accessId);

      ref.handler->provideDirTree(dirname);
    }

    // ---------------------------------------------------------------
    void
    MediaManager::releaseFile(MediaAccessId   accessId,
                              const Pathname &filename) const
    {
      MutexLock glock(g_Mutex);

      ManagedMedia &ref( m_impl->findMM(accessId));

      ref.checkAttached(accessId);

      ref.handler->releaseFile(filename);
    }

    // ---------------------------------------------------------------
    void
    MediaManager::releaseDir(MediaAccessId   accessId,
                             const Pathname &dirname) const
    {
      MutexLock glock(g_Mutex);

      ManagedMedia &ref( m_impl->findMM(accessId));

      ref.checkAttached(accessId);

      ref.handler->releaseDir(dirname);
    }


    // ---------------------------------------------------------------
    void
    MediaManager::releasePath(MediaAccessId   accessId,
                              const Pathname &pathname) const
    {
      MutexLock glock(g_Mutex);

      ManagedMedia &ref( m_impl->findMM(accessId));

      ref.checkAttached(accessId);

      ref.handler->releasePath(pathname);
    }

    // ---------------------------------------------------------------
    void
    MediaManager::dirInfo(MediaAccessId           accessId,
                          std::list<std::string> &retlist,
                          const Pathname         &dirname,
                          bool                    dots) const
    {
      MutexLock glock(g_Mutex);

      ManagedMedia &ref( m_impl->findMM(accessId));

      // FIXME: ref.checkDesired(accessId); ???
      ref.checkAttached(accessId);

      ref.handler->dirInfo(retlist, dirname, dots);
    }

    // ---------------------------------------------------------------
    void
    MediaManager::dirInfo(MediaAccessId           accessId,
                          filesystem::DirContent &retlist,
                          const Pathname         &dirname,
                          bool                    dots) const
    {
      MutexLock glock(g_Mutex);

      ManagedMedia &ref( m_impl->findMM(accessId));

      // FIXME: ref.checkDesired(accessId); ???
      ref.checkAttached(accessId);

      ref.handler->dirInfo(retlist, dirname, dots);
    }

    // ---------------------------------------------------------------
    // STATIC
    time_t
    MediaManager::getMountTableMTime()
    {
      MutexLock glock(g_Mutex);
      return MediaManager_Impl::getMountTableMTime();
    }

    // ---------------------------------------------------------------
    // STATIC
    MountEntries
    MediaManager::getMountEntries()
    {
      MutexLock glock(g_Mutex);

      return MediaManager_Impl::getMountEntries();
    }

    // ---------------------------------------------------------------
    bool
    MediaManager::isUseableAttachPoint(const Pathname &path) const
    {
      if( path.empty() || path == "/" || !PathInfo(path).isDir())
        return false;

      MutexLock glock(g_Mutex);

      //
      // check against our current attach points
      //
      ManagedMediaMap::const_iterator m(m_impl->mediaMap.begin());
      for( ; m != m_impl->mediaMap.end(); ++m)
      {
        AttachedMedia ret = m->second.handler->attachedMedia();
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
      MutexLock glock(g_Mutex);

      ManagedMedia &ref( m_impl->findMM(accessId));

      return ref.handler->attachedMedia();
    }

    // ---------------------------------------------------------------
    AttachedMedia
    MediaManager::findAttachedMedia(const MediaSourceRef &media) const
    {
      MutexLock glock(g_Mutex);

      if( !media || media->type.empty())
        return AttachedMedia();

      ManagedMediaMap::const_iterator m(m_impl->mediaMap.begin());
      for( ; m != m_impl->mediaMap.end(); ++m)
      {
        if( !m->second.handler->isAttached())
          continue;

        AttachedMedia ret = m->second.handler->attachedMedia();
        if( ret.mediaSource && ret.mediaSource->equals( *media))
            return ret;
      }
      return AttachedMedia();
    }

    // ---------------------------------------------------------------
    void
    MediaManager::forceReleaseShared(const MediaSourceRef &media)
    {
      MutexLock glock(g_Mutex);

      if( !media || media->type.empty())
        return;

      ManagedMediaMap::iterator m(m_impl->mediaMap.begin());
      for( ; m != m_impl->mediaMap.end(); ++m)
      {
        if( !m->second.handler->isAttached())
          continue;

        AttachedMedia ret = m->second.handler->attachedMedia();
        if( ret.mediaSource && ret.mediaSource->equals( *media))
        {
          m->second.handler->release(false);
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
