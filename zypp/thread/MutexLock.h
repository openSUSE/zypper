/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/thread/MutexLock.h
 *
*/
#ifndef   ZYPP_THREAD_MUTEXLOCK_H
#define   ZYPP_THREAD_MUTEXLOCK_H

#include "zypp/thread/Mutex.h"
#include <cassert>


//////////////////////////////////////////////////////////////////////
namespace zypp
{ ////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////
  namespace thread
  { //////////////////////////////////////////////////////////////////


    // -------------------------------------------------------------
    class MutexLock
    {
    public:
      explicit MutexLock(Mutex &mutex, bool init_locked=true)
        : m_mutex(&mutex)
        , m_locked(false)
      {
        if(init_locked)
          lock();
      }

      MutexLock(const MutexLock &ref)
        : m_mutex( ref.m_mutex)
        , m_locked(ref.m_locked)
      {
        ref.m_locked = false;
      }

      ~MutexLock()
      {
        try
        {
          if( m_locked)
            unlock();
        }
        catch( ... )
        {
          // don't let exceptions escape
        }
      }

      void lock()
      {
        assert(m_locked == false);
        m_mutex->lock();
        m_locked = true;
      }

      void unlock()
      {
        assert(m_locked == true);
        m_mutex->unlock();
        m_locked = false;
      }

      bool trylock()
      {
        assert(m_locked == false);
        m_locked = m_mutex->trylock();
        return m_locked;
      }

      bool locked()
      {
        return m_locked;
      }

    private:
      Mutex        *m_mutex;
      mutable bool  m_locked;
      //friend class Condition;
    };


    //////////////////////////////////////////////////////////////////
  } // namespace thread
  ////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////
} // namespace zypp
//////////////////////////////////////////////////////////////////////

#endif // ZYPP_THREAD_MUTEXLOCK_H
/*
** vim: set ts=2 sts=2 sw=2 ai et:
*/
