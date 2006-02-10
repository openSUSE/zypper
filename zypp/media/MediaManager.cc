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
#include <zypp/media/Mount.h>
//#include <zypp/media/Hal.h>
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

      /*
      ** Use a map<ID, struct { handler, verifier, isVerified }> ?
      */
      enum MediaVerifyState
      {
        NOT_VERIFIED, VERIFIED
      };

      typedef std::map<MediaAccessId, MediaVerifierRef> MediaVfyMap;
      typedef std::map<MediaAccessId, MediaAccessRef>   MediaAccMap;
      typedef std::map<MediaAccessId, MediaVerifyState> MediaVStMap;

      ////////////////////////////////////////////////////////////////
    } // anonymous
    //////////////////////////////////////////////////////////////////


    //////////////////////////////////////////////////////////////////
    class MediaManager::Impl
    {
    private:
      time_t        mtab_mtime;
      MountEntries  mtab_table;
      MediaAccessId last_accessid;

    public:
      MediaVfyMap  mediaVfyMap;
      MediaAccMap  mediaAccMap;
      MediaVStMap  mediaVStMap;

      Impl()
        : mtab_mtime(0)
        , last_accessid(0)
      {}

      ~Impl()
      {}

      MediaAccessId
      nextAccessId()
      {
        return ++last_accessid;
      }

      bool hasMediaAcc(MediaAccessId accessId) const
      {
        return mediaAccMap.find(accessId) != mediaAccMap.end();
      }

      bool hasVerifier(MediaAccessId accessId) const
      {
        return mediaVfyMap.find(accessId) != mediaVfyMap.end();
      }

      MountEntries
      getMountEntries()
      {
        if( mtab_mtime == 0 ||
            mtab_mtime != zypp::PathInfo("/etc/mtab").mtime())
        {
          mtab_table = Mount::getEntries("/etc/mtab");
        }
        return mtab_table;
      }
    };


    //////////////////////////////////////////////////////////////////
    // STATIC
    zypp::RW_pointer<MediaManager::Impl> MediaManager::m_impl(NULL);


    //////////////////////////////////////////////////////////////////
    MediaManager::MediaManager()
    {
      MutexLock glock(g_Mutex);
      if( !m_impl)
      {
        m_impl.reset( new MediaManager::Impl());
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
      MediaAccessRef   accRef( new MediaAccess());
      MediaVerifierRef vfyRef( new NoVerifier());

      accRef->open(url, preferred_attach_point);

      MediaAccessId nextId = m_impl->nextAccessId();

      m_impl->mediaAccMap[nextId] = accRef;
      m_impl->mediaVfyMap[nextId] = vfyRef;
      m_impl->mediaVStMap[nextId] = NOT_VERIFIED;

      DBG << "Opened new media access using id " << nextId
          << " to " << url.asString() << std::endl;
      return nextId;
    }

    // ---------------------------------------------------------------
    /*
    void
    MediaManager::reopen(MediaAccessId accessId, const Url &url,
                         const Pathname & preferred_attach_point)
    {
      MutexLock glock(g_Mutex);

      if( !m_impl->hasMediaAcc(accessId))
      {
        ZYPP_THROW(MediaNotOpenException(
          "Invalid media access id " + str::numstring(accessId)
        ));
      }

      MediaAccessRef   accRef( new MediaAccess());

      accRef->open(url, preferred_attach_point);

      // release and close the old one
      m_impl->mediaAccMap[accessId]->close();

      // assign new one
      m_impl->mediaAccMap[accessId] = accRef;
      m_impl->mediaVStMap[accessId] = NOT_VERIFIED;

      DBG << "Reopened media access id " << accessId
          << " to " << url.asString() << std::endl;
    }
    */
    
    // ---------------------------------------------------------------
    /*
    bool
    MediaManager::swap(MediaAccessId idOne, MediaAccessId idTwo)
    {
      MutexLock glock(g_Mutex);

      if( m_impl->hasMediaAcc(idOne) &&  m_impl->hasMediaAcc(idTwo))
      {
        // FIXME: swich verifier and its state as well
        MediaAccessRef tmp( m_impl->mediaAccMap[idOne]);

        m_impl->mediaAccMap[idOne] = m_impl->mediaAccMap[idTwo];
        m_impl->mediaAccMap[idTwo] = tmp;

        DBG << "Swapped media access ids "
            << idOne << " and " << idTwo << std::endl;
        return true;
      }
      return false;
    }
    */

    // ---------------------------------------------------------------
    void
    MediaManager::close(MediaAccessId accessId)
    {
      MutexLock glock(g_Mutex);

      if( !m_impl->hasMediaAcc(accessId))
      {
        ZYPP_THROW(MediaNotOpenException(
          "Invalid media access id " + str::numstring(accessId)
        ));
      }

      m_impl->mediaAccMap[accessId]->close();
      m_impl->mediaAccMap.erase(accessId);
      m_impl->mediaVfyMap.erase(accessId);
      m_impl->mediaVStMap.erase(accessId);
    }

    // ---------------------------------------------------------------
    bool
    MediaManager::isOpen(MediaAccessId accessId) const
    {
      MutexLock glock(g_Mutex);

      return m_impl->hasMediaAcc(accessId) &&
             m_impl->mediaAccMap[accessId]->isOpen();
    }

    // ---------------------------------------------------------------
    std::string
    MediaManager::protocol(MediaAccessId accessId) const
    {
      MutexLock glock(g_Mutex);

      if( m_impl->hasMediaAcc(accessId))
        return m_impl->mediaAccMap[accessId]->protocol();
      else
        return std::string("unknown");
    }

    // ---------------------------------------------------------------
    Url
    MediaManager::url(MediaAccessId accessId) const
    {
      MutexLock glock(g_Mutex);

      if( m_impl->hasMediaAcc(accessId))
        return m_impl->mediaAccMap[accessId]->url();
      else
        return Url();
    }

    // ---------------------------------------------------------------
    void
    MediaManager::addVerifier(MediaAccessId           accessId,
                              const MediaVerifierRef &verifier)
    {
      MutexLock glock(g_Mutex);

      if( !verifier)
        ZYPP_THROW(MediaException("Invalid verifier reference"));

      if( !m_impl->hasMediaAcc(accessId))
      {
        ZYPP_THROW(MediaNotOpenException(
          "Invalid media access id " + str::numstring(accessId)
        ));
      }

      m_impl->mediaVfyMap[accessId] = verifier;
    }

    // ---------------------------------------------------------------
    void
    MediaManager::delVerifier(MediaAccessId accessId)
    {
      MutexLock glock(g_Mutex);

      if( !m_impl->hasMediaAcc( accessId))
      {
        ZYPP_THROW(MediaNotOpenException(
          "Invalid media access id " + str::numstring(accessId)
        ));
      }

      MediaVerifierRef vfyRef( new NoVerifier());
      m_impl->mediaVfyMap[accessId] = vfyRef;
    }

    // ---------------------------------------------------------------
    void
    MediaManager::attach(MediaAccessId accessId, bool next)
    {
      MutexLock glock(g_Mutex);

      if( !m_impl->hasMediaAcc( accessId))
      {
        ZYPP_THROW(MediaNotOpenException(
          "Invalid media access id " + str::numstring(accessId)
        ));
      }

      m_impl->mediaAccMap[accessId]->attach(next);
    }

    // ---------------------------------------------------------------
#if 0
    void
    MediaManager::attachDesiredMedia(MediaAccessId accessId,
                                     bool          eject)
    {
      MutexLock glock(g_Mutex);

      if( !m_impl->hasMediaAcc( accessId))
      {
        ZYPP_THROW(MediaNotOpenException(
          "Invalid media access id " + str::numstring(accessId)
        ));
      }

      // this should never happen, since we allways add a NoVerifier!
      if( !m_impl->hasVerifier( accessId))
        ZYPP_THROW(MediaException("Invalid verifier detected"));

      // check the attached media first..
      if( m_impl->mediaAccMap[accessId]->isAttached())
      {
        bool desired;
        try {
          desired = m_impl->mediaVfyMap[accessId]->isDesiredMedia(
            m_impl->mediaAccMap[accessId]
          );
        }
        catch(const zypp::Exception &e) {
          ZYPP_CAUGHT(e);
          desired = false;
        }

        if( desired) {
          // we found it already attached
          return;
        }
        else {
          try
          {
            m_impl->mediaAccMap[accessId]->release(eject);
          }
          /*
          ** FIXME: remember it for later
          **
          catch(const zypp::MediaIsSharedException &e) {
            ZYPP_CAUGHT(e);
          }
          */
          // allow to escape umount exceptions
        }
      }

      // FIXME: implement it.
    }
#endif

    // ---------------------------------------------------------------
    void
    MediaManager::release(MediaAccessId accessId, bool eject)
    {
      MutexLock glock(g_Mutex);

      if( !m_impl->hasMediaAcc( accessId))
      {
        ZYPP_THROW(MediaNotOpenException(
          "Invalid media access id " + str::numstring(accessId)
        ));
      }

      m_impl->mediaAccMap[accessId]->release(eject);
    }

    // ---------------------------------------------------------------
    void
    MediaManager::disconnect(MediaAccessId accessId)
    {
      MutexLock glock(g_Mutex);

      if( !m_impl->hasMediaAcc( accessId))
      {
        ZYPP_THROW(MediaNotOpenException(
          "Invalid media access id " + str::numstring(accessId)
        ));
      }

      return m_impl->mediaAccMap[accessId]->disconnect();
    }

    // ---------------------------------------------------------------
    bool
    MediaManager::isAttached(MediaAccessId accessId) const
    {
      MutexLock glock(g_Mutex);

      if( !m_impl->hasMediaAcc( accessId))
      {
        // FIXME: throw or just return false?
        ZYPP_THROW(MediaNotOpenException(
          "Invalid media access id " + str::numstring(accessId)
        ));
      }

      return m_impl->mediaAccMap[accessId]->isAttached();
    }

    // ---------------------------------------------------------------
    bool
    MediaManager::isDesiredMedia(MediaAccessId accessId) const
    {
      MutexLock glock(g_Mutex);

      if( !isAttached(accessId))
        return false;

      // this should never happen, since we allways add a NoVerifier!
      if( !m_impl->hasVerifier( accessId))
        ZYPP_THROW(MediaException("Invalid verifier detected"));

      bool ok;
      try {
        ok = m_impl->mediaVfyMap[accessId]->isDesiredMedia(
          m_impl->mediaAccMap[accessId]
        );
      }
      catch(const zypp::Exception &e) {
          ZYPP_CAUGHT(e);
          ok = false;
      }
      return ok;
    }

    // ---------------------------------------------------------------
    bool
    MediaManager::isDesiredMedia(MediaAccessId           accessId,
                                 const MediaVerifierRef &verifier) const
    {
      MutexLock glock(g_Mutex);

      if( !isAttached(accessId))
        return false;

      MediaVerifierRef v( verifier);
      if( !v)
        ZYPP_THROW(MediaException("Invalid verifier reference"));

      bool ok;
      try {
        ok = v->isDesiredMedia(
          m_impl->mediaAccMap[accessId]
        );
      }
      catch(const zypp::Exception &e) {
          ZYPP_CAUGHT(e);
          ok = false;
      }
      return ok;
    }

    // ---------------------------------------------------------------
    Pathname
    MediaManager::localRoot(MediaAccessId accessId) const
    {
      MutexLock glock(g_Mutex);

      if( !m_impl->hasMediaAcc( accessId))
      {
        // FIXME: throw or just return false?
        ZYPP_THROW(MediaNotOpenException(
          "Invalid media access id " + str::numstring(accessId)
        ));
      }

      Pathname path;
      path = m_impl->mediaAccMap[accessId]->localRoot();
      return path;
    }

    // ---------------------------------------------------------------
    Pathname
    MediaManager::localPath(MediaAccessId accessId,
                            const Pathname & pathname) const
    {
      MutexLock glock(g_Mutex);

      if( !m_impl->hasMediaAcc( accessId))
      {
        ZYPP_THROW(MediaNotOpenException(
          "Invalid media access id " + str::numstring(accessId)
        ));
      }

      Pathname path;
      path = m_impl->mediaAccMap[accessId]->localPath(pathname);
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

      // FIXME: cache the results
      /*
      if( !isDesiredMedia(accessId))
      {
        ZYPP_THROW(MediaNotDesiredException(
          m_impl->mediaAccMap[accessId]->url()
        ));
      }
      */

      m_impl->mediaAccMap[accessId]->provideFile(filename, cached, checkonly);
    }

    // ---------------------------------------------------------------
    void
    MediaManager::provideDir(MediaAccessId   accessId,
                             const Pathname &dirname) const
    {
      MutexLock glock(g_Mutex);

      // FIXME: cache the results
      /*
      if( !isDesiredMedia(accessId))
      {
        ZYPP_THROW(MediaNotDesiredException(
          m_impl->mediaAccMap[accessId]->url()
        ));
      }
      */

      m_impl->mediaAccMap[accessId]->provideDir(dirname);
    }

    // ---------------------------------------------------------------
    void
    MediaManager::provideDirTree(MediaAccessId   accessId,
                                 const Pathname &dirname) const
    {
      MutexLock glock(g_Mutex);

      // FIXME: cache the results
      /*
      if( !isDesiredMedia(accessId))
      {
        ZYPP_THROW(MediaNotDesiredException(
          m_impl->mediaAccMap[accessId]->url()
        ));
      }
      */

      m_impl->mediaAccMap[accessId]->provideDirTree(dirname);
    }

    // ---------------------------------------------------------------
    void
    MediaManager::releaseFile(MediaAccessId   accessId,
                              const Pathname &filename) const
    {
      MutexLock glock(g_Mutex);

      // FIXME: check isDesiredMedia(accessId) ???
      if( !m_impl->hasMediaAcc( accessId))
      {
        ZYPP_THROW(MediaNotOpenException(
          "Invalid media access id " + str::numstring(accessId)
        ));
      }

      m_impl->mediaAccMap[accessId]->releaseFile(filename);
    }

    // ---------------------------------------------------------------
    void
    MediaManager::releaseDir(MediaAccessId   accessId,
                             const Pathname &dirname) const
    {
      MutexLock glock(g_Mutex);

      // FIXME: check isDesiredMedia(accessId) ???
      if( !m_impl->hasMediaAcc( accessId))
      {
        ZYPP_THROW(MediaNotOpenException(
          "Invalid media access id " + str::numstring(accessId)
        ));
      }

      m_impl->mediaAccMap[accessId]->releaseDir(dirname);
    }


    // ---------------------------------------------------------------
    void
    MediaManager::releasePath(MediaAccessId   accessId,
                              const Pathname &pathname) const
    {
      MutexLock glock(g_Mutex);

      // FIXME: check isDesiredMedia(accessId) ???
      if( !m_impl->hasMediaAcc( accessId))
      {
        ZYPP_THROW(MediaNotOpenException(
          "Invalid media access id " + str::numstring(accessId)
        ));
      }

      m_impl->mediaAccMap[accessId]->releasePath(pathname);
    }

    // ---------------------------------------------------------------
    void
    MediaManager::dirInfo(MediaAccessId           accessId,
                          std::list<std::string> &retlist,
                          const Pathname         &dirname,
                          bool                    dots) const
    {
      MutexLock glock(g_Mutex);

      // FIXME: check isDesiredMedia(accessId) ???
      if( !m_impl->hasMediaAcc( accessId))
      {
        ZYPP_THROW(MediaNotOpenException(
          "Invalid media access id " + str::numstring(accessId)
        ));
      }

      m_impl->mediaAccMap[accessId]->dirInfo(retlist, dirname, dots);
    }

    // ---------------------------------------------------------------
    void
    MediaManager::dirInfo(MediaAccessId           accessId,
                          filesystem::DirContent &retlist,
                          const Pathname         &dirname,
                          bool                    dots) const
    {
      MutexLock glock(g_Mutex);

      // FIXME: check isDesiredMedia(accessId) ???
      if( !m_impl->hasMediaAcc( accessId))
      {
        ZYPP_THROW(MediaNotOpenException(
          "Invalid media access id " + str::numstring(accessId)
        ));
      }

      m_impl->mediaAccMap[accessId]->dirInfo(retlist, dirname, dots);
    }


    // ---------------------------------------------------------------
    AttachedMedia
    MediaManager::findAttachedMedia(const MediaSourceRef &media) const
    {
      MutexLock glock(g_Mutex);

      if( !media || media->type.empty())
        return AttachedMedia();

      MediaAccMap::const_iterator a(m_impl->mediaAccMap.begin());
      for( ; a != m_impl->mediaAccMap.end(); ++a)
      {
        if( !a->second->isAttached())
          continue;

        AttachedMedia ret = a->second->attachedMedia();
        if( ret.mediaSource && ret.mediaSource->equals( *media))
            return ret;
      }
      return AttachedMedia();
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
