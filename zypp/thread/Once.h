/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/thread/Once.h
 *
*/
#ifndef   ZYPP_THREAD_ONCE_H
#define   ZYPP_THREAD_ONCE_H

#include <pthread.h>


//////////////////////////////////////////////////////////////////////
namespace zypp
{ ////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////
  namespace thread
  { //////////////////////////////////////////////////////////////////


    /** The initialization value for OnceFlag variables. */
    #define ZYPP_ONCE_INIT  PTHREAD_ONCE_INIT


    /** The OnceFlag variable type.
     */
    typedef pthread_once_t OnceFlag;


    /**
     * Call once function.
     *
     * The purpose of callOnce is to ensure that a piece of initialization
     * code is executed at most once.
     * The OnceFlag \p flag has to point to a static or extern variable,
     * that was statically initialized to ZYPP_ONCE_INIT.
     *
     * The first time callOnce is called with a given \p onceFlag argument,
     * it calls \p fuct with no argument and changes the value of \p flag
     * to indicate that the function has been run.
     * Subsequent calls with the same once flag does nothing.
     */
    void callOnce(OnceFlag& flag, void (*func)());

    inline void callOnce(OnceFlag& flag, void (*func)())
    {
      pthread_once(&flag, func);
    }

    //////////////////////////////////////////////////////////////////
  } // namespace thread
  ////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////
} // namespace zypp
//////////////////////////////////////////////////////////////////////

#endif // ZYPP_THREAD_ONCE_H
/*
** vim: set ts=2 sts=2 sw=2 ai et:
*/
