/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/thread/Mutex.h
 */
#ifndef   ZYPP_THREAD_MUTEX_H
#define   ZYPP_THREAD_MUTEX_H

#include "zypp/base/NonCopyable.h"
#include "zypp/thread/MutexException.h"
#include <pthread.h>

//////////////////////////////////////////////////////////////////////
namespace zypp
{ ////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////
  namespace thread
  { //////////////////////////////////////////////////////////////////


    typedef pthread_mutex_t RecursiveMutex_t;


    ////////////////////////////////////////////////////////////////
    //
    // CLASS NAME : Mutex
    //
    /** A recursive Mutex.
     */
    class Mutex: public zypp::base::NonCopyable
    {
    public:
      /** Create a new recursive Mutex object.
       * \throws MutexException on initialization failure.
       */
      Mutex();

      /** Destroys this Mutex object.
       */
      ~Mutex();

      /** Acquire ownership of this Mutex object.
       * This call will block if another thread has ownership of
       * this Mutex. When it returns, the current thread is the
       * owner of this Mutex object.
       *
       * In the same thread, this recursive mutex can be acquired
       * multiple times.
       *
       * \throws MutexException if the maximum number of recursive
       *         locks for mutex has been exceeded.
       */
      void lock();

      /** Release ownership of this Mutex object.
       * If another thread is waiting to acquire the ownership of
       * this mutex it will stop blocking and acquire ownership
       * when this call returns.
       *
       * \throws MutexException if the current thread does not
       *         own the mutex.
       */
      void unlock();

      /** Try to acquire ownership of this Mutex object.
       * This call will return false if another thread has ownership
       * of this Mutex or the maximum number of recursive locks for
       * mutex has been exceeded.
       * When it returns true, the current thread is the owner of
       * this Mutex object.
       *
       * \return true, if ownership was acquired.
       */
      bool trylock();

    private:
      RecursiveMutex_t m_mutex;
    };


    //////////////////////////////////////////////////////////////////
  } // namespace thread
  ////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////
} // namespace zypp
//////////////////////////////////////////////////////////////////////

#endif // ZYPP_THREAD_MUTEX_H
/*
** vim: set ts=2 sts=2 sw=2 ai et:
*/
