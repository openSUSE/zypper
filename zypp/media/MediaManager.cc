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
//#include <zypp/media/Mount.h>
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

      typedef std::map<MediaId, MediaVerifierRef> MediaVfyMap;
      typedef std::map<MediaId, MediaAccessRef>   MediaAccMap;


      ////////////////////////////////////////////////////////////////
    } // anonymous
    //////////////////////////////////////////////////////////////////


    // ---------------------------------------------------------------
    class MediaManager::Impl
    {
    private:
    /*
      time_t       mtab_mtime;
      MountEntries mtab_table;
    */
      MediaId      last_mediaid;

    public:
      MediaVfyMap  mediaVfyMap;
      MediaAccMap  mediaAccMap;

      Impl()
        : /* mtab_mtime(0)
        , */ last_mediaid(0)
      {}

      ~Impl()
      {}

      MediaId
      nextMediaId()
      {
        return last_mediaid++;
      }

      bool hasMediaAcc(MediaId mediaId) const
      {
        return mediaAccMap.find(mediaId) != mediaAccMap.end();
      }

      bool hasVerifier(MediaId mediaId) const
      {
        return mediaVfyMap.find(mediaId) != mediaVfyMap.end();
      }

      /*
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
      */
    };


    // ---------------------------------------------------------------
    // STATIC
    zypp::RW_pointer<MediaManager::Impl> MediaManager::m_impl(NULL);


    // ---------------------------------------------------------------
    MediaManager::MediaManager()
    {
      MutexLock lock(g_Mutex);
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
    MediaId
    MediaManager::open(const Url &url, const Pathname &preferred_attach_point)
    {
      MutexLock lock(g_Mutex);

      // check if we already have this url
      MediaAccMap::const_iterator a(m_impl->mediaAccMap.begin());
      for( ; a != m_impl->mediaAccMap.end(); ++a)
      {
        // FIXME: not sufficient. each handler should provide
        //        method to compare its type of media url
        //        and MediaAccess to choose right handler...
        if( a->second->url().asString() == url.asString())
        {
          return a->first;
        }
      }

      // create new access handler for it
      MediaAccessRef accRef( new MediaAccess());
      MediaId        nextId( m_impl->nextMediaId());

      m_impl->mediaAccMap[nextId] = accRef;

      accRef->open(url, preferred_attach_point);

      return nextId;
    }

    // ---------------------------------------------------------------
    void
    MediaManager::close(MediaId mediaId)
    {
      MutexLock lock(g_Mutex);

      if( !m_impl->hasMediaAcc( mediaId))
        ZYPP_THROW(MediaException("Invalid media id"));

      if( m_impl->hasVerifier( mediaId))
        m_impl->mediaVfyMap.erase(mediaId);

      m_impl->mediaAccMap[mediaId]->close();
      m_impl->mediaAccMap.erase(mediaId);
    }

    // ---------------------------------------------------------------
    bool
    MediaManager::isOpen(MediaId mediaId) const
    {
      MutexLock lock(g_Mutex);

      return m_impl->hasMediaAcc( mediaId);
    }

    // ---------------------------------------------------------------
    std::string
    MediaManager::protocol(MediaId mediaId) const
    {
      MutexLock lock(g_Mutex);

      if( m_impl->hasMediaAcc( mediaId))
        return m_impl->mediaAccMap[mediaId]->protocol();
      else
        return std::string("unknown");
    }

    // ---------------------------------------------------------------
    Url
    MediaManager::url(MediaId mediaId) const
    {
      MutexLock lock(g_Mutex);

      if( m_impl->hasMediaAcc( mediaId))
        return m_impl->mediaAccMap[mediaId]->url();
      else
        return Url();
    }

    // ---------------------------------------------------------------
    void
    MediaManager::addVerifier(MediaId mediaId, const MediaVerifierRef &ref)
    {
      MutexLock lock(g_Mutex);

      if( !ref)
        ZYPP_THROW(MediaException("Invalid (empty) verifier reference"));

      if( !m_impl->hasMediaAcc( mediaId))
        ZYPP_THROW(MediaException("Invalid media id"));

      // FIXME: just replace?
      if( m_impl->hasVerifier( mediaId))
        ZYPP_THROW(MediaException("Remove verifier first"));

      m_impl->mediaVfyMap[mediaId] = ref;
    }

    // ---------------------------------------------------------------
    void
    MediaManager::delVerifier(MediaId mediaId)
    {
      MutexLock lock(g_Mutex);

      if( !m_impl->hasMediaAcc( mediaId))
        ZYPP_THROW(MediaException("Invalid media id"));

      if( m_impl->hasVerifier( mediaId))
        m_impl->mediaVfyMap.erase(mediaId);
    }

    // ---------------------------------------------------------------
    void
    MediaManager::attach(MediaId mediaId, bool next)
    {
      MutexLock lock(g_Mutex);

      if( !m_impl->hasMediaAcc( mediaId))
      {
        ZYPP_THROW(MediaNotOpenException(
          "Invalid media id " + str::numstring(mediaId)
        ));
      }

      if( !m_impl->hasVerifier( mediaId))
        ZYPP_THROW(MediaException("Add a verifier first"));

      m_impl->mediaAccMap[mediaId]->attach(next);
    }

    // ---------------------------------------------------------------
    void
    MediaManager::release(MediaId mediaId, bool eject)
    {
      MutexLock lock(g_Mutex);

      if( !m_impl->hasMediaAcc( mediaId))
      {
        ZYPP_THROW(MediaNotOpenException(
          "Invalid media id " + str::numstring(mediaId)
        ));
      }

      m_impl->mediaAccMap[mediaId]->release(eject);
    }

    // ---------------------------------------------------------------
    void
    MediaManager::disconnect(MediaId mediaId)
    {
      MutexLock lock(g_Mutex);

      if( !m_impl->hasMediaAcc( mediaId))
      {
        ZYPP_THROW(MediaNotOpenException(
          "Invalid media id " + str::numstring(mediaId)
        ));
      }
      return m_impl->mediaAccMap[mediaId]->disconnect();
    }

    // ---------------------------------------------------------------
    bool
    MediaManager::isAttached(MediaId mediaId) const
    {
      MutexLock lock(g_Mutex);

      if( !m_impl->hasMediaAcc( mediaId))
      {
        // FIXME: throw or just return false?
        ZYPP_THROW(MediaNotOpenException(
          "Invalid media id " + str::numstring(mediaId)
        ));
      }
      return m_impl->mediaAccMap[mediaId]->isAttached();
    }

    // ---------------------------------------------------------------
    bool
    MediaManager::isDesiredMedia(MediaId mediaId, MediaNr mediaNr) const
    {
      MutexLock lock(g_Mutex);

      if( !isAttached(mediaId))
        return false;

      // FIXME: throw or just return false?
      if( !m_impl->hasVerifier( mediaId))
        ZYPP_THROW(MediaException("Add a verifier first"));

      bool ok;
      try {
        ok = m_impl->mediaVfyMap[mediaId]->isDesiredMedia(
          m_impl->mediaAccMap[mediaId], mediaNr
        );
      }
      catch( ... ) { ok = false; }
      return ok;
    }

    // ---------------------------------------------------------------
    Pathname
    MediaManager::localRoot(MediaId mediaId) const
    {
      MutexLock lock(g_Mutex);

      if( !m_impl->hasMediaAcc( mediaId))
      {
        ZYPP_THROW(MediaNotOpenException(
          "Invalid media id " + str::numstring(mediaId)
        ));
      }
      Pathname path;
      path = m_impl->mediaAccMap[mediaId]->localRoot();
      return path;
    }

    // ---------------------------------------------------------------
    Pathname
    MediaManager::localPath(MediaId mediaId,
                            const Pathname & pathname) const
    {
      MutexLock lock(g_Mutex);

      if( !m_impl->hasMediaAcc( mediaId))
      {
        ZYPP_THROW(MediaNotOpenException(
          "Invalid media id " + str::numstring(mediaId)
        ));
      }
      Pathname path;
      path = m_impl->mediaAccMap[mediaId]->localPath(pathname);
      return path;
    }

    // ---------------------------------------------------------------
    void
    MediaManager::provideFile(MediaId mediaId, MediaNr mediaNr,
                              const Pathname &filename,
                              bool cached, bool checkonly) const
    {
      MutexLock lock(g_Mutex);

      if( !isDesiredMedia(mediaId, mediaNr))
      {
        ZYPP_THROW(MediaNotDesiredException(
          m_impl->mediaAccMap[mediaId]->url(), mediaNr
        ));
      }

      m_impl->mediaAccMap[mediaId]->provideFile(filename, cached, checkonly);
    }

    // ---------------------------------------------------------------
    void
    MediaManager::provideDir(MediaId mediaId,
                             MediaNr mediaNr,
                             const Pathname & dirname ) const
    {
      MutexLock lock(g_Mutex);

      if( !isDesiredMedia(mediaId, mediaNr))
      {
        ZYPP_THROW(MediaNotDesiredException(
          m_impl->mediaAccMap[mediaId]->url(), mediaNr
        ));
      }

      m_impl->mediaAccMap[mediaId]->provideDir(dirname);
    }

    // ---------------------------------------------------------------
    void
    MediaManager::provideDirTree(MediaId mediaId,
                                 MediaNr mediaNr,
                                 const Pathname & dirname ) const
    {
      MutexLock lock(g_Mutex);

      if( !isDesiredMedia(mediaId, mediaNr))
      {
        ZYPP_THROW(MediaNotDesiredException(
          m_impl->mediaAccMap[mediaId]->url(), mediaNr
        ));
      }

      m_impl->mediaAccMap[mediaId]->provideDirTree(dirname);
    }

    // ---------------------------------------------------------------
    void
    MediaManager::releaseFile(MediaId mediaId,
                              const Pathname & filename) const
    {
      MutexLock lock(g_Mutex);

      if( !m_impl->hasMediaAcc( mediaId))
      {
        ZYPP_THROW(MediaNotOpenException(
          "Invalid media id " + str::numstring(mediaId)
        ));
      }

      m_impl->mediaAccMap[mediaId]->releaseFile(filename);
    }

    // ---------------------------------------------------------------
    void
    MediaManager::releaseDir(MediaId mediaId,
                             const Pathname & dirname) const
    {
      MutexLock lock(g_Mutex);

      if( !m_impl->hasMediaAcc( mediaId))
      {
        ZYPP_THROW(MediaNotOpenException(
          "Invalid media id " + str::numstring(mediaId)
        ));
      }

      m_impl->mediaAccMap[mediaId]->releaseDir(dirname);
    }


    // ---------------------------------------------------------------
    void
    MediaManager::releasePath(MediaId mediaId,
                              const Pathname & pathname) const
    {
      MutexLock lock(g_Mutex);

      if( !m_impl->hasMediaAcc( mediaId))
      {
        ZYPP_THROW(MediaNotOpenException(
          "Invalid media id " + str::numstring(mediaId)
        ));
      }

      m_impl->mediaAccMap[mediaId]->releasePath(pathname);
    }

    // ---------------------------------------------------------------
    void
    MediaManager::dirInfo(MediaId mediaId,
                          std::list<std::string> & retlist,
                          const Pathname & dirname, bool dots) const
    {
      MutexLock lock(g_Mutex);

      if( !m_impl->hasMediaAcc( mediaId))
      {
        ZYPP_THROW(MediaNotOpenException(
          "Invalid media id " + str::numstring(mediaId)
        ));
      }

      m_impl->mediaAccMap[mediaId]->dirInfo(retlist, dirname, dots);
    }

    // ---------------------------------------------------------------
    void
    MediaManager::dirInfo(MediaId mediaId,
                          filesystem::DirContent & retlist,
                          const Pathname & dirname, bool dots) const
    {
      MutexLock lock(g_Mutex);

      if( !m_impl->hasMediaAcc( mediaId))
      {
        ZYPP_THROW(MediaNotOpenException(
          "Invalid media id " + str::numstring(mediaId)
        ));
      }

      m_impl->mediaAccMap[mediaId]->dirInfo(retlist, dirname, dots);
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
