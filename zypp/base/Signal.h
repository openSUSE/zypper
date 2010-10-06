/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/Signal.h
 *
*/
#ifndef ZYPP_BASE_SIGNAL_H
#define ZYPP_BASE_SIGNAL_H

#include <csignal>
#include <iosfwd>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  /** Exception safe signal handler save/restore.
   * \ingroup g_RAII
   */
  class SignalSaver
  {
    public:
      SignalSaver( int signum_r, sighandler_t handler_r )
      : _signum( signum_r )
      { _orighandler = ::signal( signum_r, handler_r ); }
      ~SignalSaver()
      { ::signal( _signum, _orighandler ); }
    private:
      int _signum;
      ::sighandler_t _orighandler;
  };

  /** Exception safe sigprocmask save/restore.
   * \ingroup g_RAII
   */
  class SigprocmaskSaver
  {
    public:
      /** Ctor saving the original sigprocmask. */
      SigprocmaskSaver()
      { ::sigprocmask( SIG_SETMASK, NULL, &_origmask ); }
      /** Dtor restoring the original sigprocmask. */
      ~SigprocmaskSaver()
      { ::sigprocmask( SIG_SETMASK, &_origmask, NULL ); }
    public:
      /** Temporary block signal. */
      void block( int signum_r )
      {
	::sigset_t mask;
	::sigemptyset( & mask );
	::sigaddset( & mask, signum_r );
	::sigprocmask( SIG_BLOCK, &mask, NULL );
      }
      /** Temporary unblock signal. */
      void unblock( int signum_r )
      {
	::sigset_t mask;
	::sigemptyset( & mask );
	::sigaddset( & mask, signum_r );
	::sigprocmask( SIG_UNBLOCK, &mask, NULL );
      }
      /** Whether signal delivery is pending. */
      bool pending( int signum_r )
      {
	::sigset_t mask;
	::sigpending( &mask );
	return ::sigismember( &mask, signum_r );
      }
      /** Wait for signals not blocked in original sigprocmask. */
      void suspend()
      { ::sigsuspend( &_origmask ); }
    private:
      ::sigset_t _origmask;
  };

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_SIGNAL_H
