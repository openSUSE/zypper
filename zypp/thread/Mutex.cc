/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/thread/Mutex.cc
 */
#include "zypp/thread/Mutex.h"
#include "zypp/thread/MutexException.h"
#include "zypp/base/Gettext.h"


//////////////////////////////////////////////////////////////////////
namespace zypp
{ ////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////
  namespace thread
  { //////////////////////////////////////////////////////////////////

    // -------------------------------------------------------------
    Mutex::Mutex()
    {
      pthread_mutexattr_t attr;

      int ret = pthread_mutexattr_init(&attr);
      if( ret != 0)
      {
        ZYPP_THROW_ERRNO_MSG(zypp::thread::MutexException,
        _("Can't initialize mutex attributes"));
      }

      ret = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
      if( ret != 0)
      {
        ZYPP_THROW_ERRNO_MSG(MutexException,
        _("Can't set recursive mutex attribute"));
      }

      ret = pthread_mutex_init(&m_mutex, &attr);
      if( ret != 0)
      {
        ZYPP_THROW_ERRNO_MSG(MutexException,
        _("Can't initialize recursive mutex"));
      }
    }

    // -------------------------------------------------------------
    Mutex::~Mutex()
    {
      if( pthread_mutex_destroy(&m_mutex) != 0 && errno == EBUSY)
      {
        // try to unlock and to destroy again...
        if( pthread_mutex_unlock(&m_mutex) == 0)
        {
            pthread_mutex_destroy(&m_mutex);
        }
        /*
        else
        {
          ZYPP_THROW_ERRNO_MSG(MutexException,
          _("Can't destroy mutex owned by another thread"));
        }
        */
      }
    }
 
    // -------------------------------------------------------------
    void Mutex::lock()
    {
      if( pthread_mutex_lock(&m_mutex) != 0)
      {
        ZYPP_THROW_ERRNO_MSG(MutexException,
        _("Can't acquire the mutex lock"));
      }
    }

    // -------------------------------------------------------------
    void Mutex::unlock()
    {
      if( pthread_mutex_unlock(&m_mutex) != 0)
      {
        ZYPP_THROW_ERRNO_MSG(MutexException,
        _("Can't release the mutex lock"));
      }
    }

    // -------------------------------------------------------------
    bool Mutex::trylock()
    {
      return (pthread_mutex_trylock(&m_mutex) == 0);
    }


    //////////////////////////////////////////////////////////////////
  } // namespace thread
  ////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////
} // namespace zypp
//////////////////////////////////////////////////////////////////////

/*
** vim: set ts=2 sts=2 sw=2 ai et:
*/
