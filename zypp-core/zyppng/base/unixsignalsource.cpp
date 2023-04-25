#include "unixsignalsource.h"
#include "private/abstracteventsource_p.h"
#include <zypp-core/AutoDispose.h>
#include <zypp-core/base/Errno.h>
#include <zypp-core/base/Logger.h>
#include <unordered_map>

#include <sys/signalfd.h>
#include <signal.h>

namespace zyppng {

  class UnixSignalSourcePrivate : public AbstractEventSourcePrivate
  {
    ZYPP_DECLARE_PUBLIC(UnixSignalSource)
  public:
    UnixSignalSourcePrivate( UnixSignalSource &p ) : AbstractEventSourcePrivate(p) {
      ::sigemptyset   ( &_orgSigMask );
      pthread_sigmask (SIG_SETMASK, nullptr, &_orgSigMask);
    }

    sigset_t     _orgSigMask;
    zypp::AutoFD _signalFd;
    std::unordered_map<int, int> _signalRefCount;
    Signal<void (int)> _sigReceived;
  };

  ZYPP_IMPL_PRIVATE (UnixSignalSource);

  UnixSignalSource::UnixSignalSource() : AbstractEventSource( *(new UnixSignalSourcePrivate(*this)) )
  {
  }

  UnixSignalSource::~UnixSignalSource()
  {
    Z_D();
    // restore the original sigmask
    pthread_sigmask (SIG_SETMASK, &d->_orgSigMask, nullptr);
  }

  UnixSignalSourceRef UnixSignalSource::create()
  {
    return UnixSignalSourceRef( new UnixSignalSource() );
  }

  bool UnixSignalSource::addSignal(int signum )
  {
    Z_D();

    if ( d->_signalRefCount.count(signum) != 0 &&  d->_signalRefCount[signum] > 0 ) {
      // we already handle this signal, just increase refcount
      d->_signalRefCount[signum]++;
    } else {

      const auto &handleError = [&]() {
        d->_signalRefCount.erase ( signum );
        return false;
      };

      // add the signal to our map
      d->_signalRefCount[signum] = 1;

      sigset_t     sigMask;
      sigemptyset (&sigMask );

      // add all the signals we monitor to our set so we can update the signalfd correctly
      for ( const auto &sig : d->_signalRefCount ) {
        sigaddset(&sigMask, sig.first);
      }

      // signalfd signals should be blocked
      // man page says: The set of blocked signals is the union of the current set and the set argument.
      // so we should not accidentially delete other blocks done by the application
      if (pthread_sigmask (SIG_BLOCK, &sigMask, NULL) == -1) {
        return handleError();
      }


      // set or update our signal fd
      zypp::AutoFD aFd = signalfd ( d->_signalFd, &sigMask, SFD_NONBLOCK | SFD_CLOEXEC );
      if ( aFd == -1 ){
        return handleError();
      }

      if ( d->_signalFd != aFd ) {
        d->_signalFd = aFd;
      } else {
        aFd.resetDispose ();
      }
      updateFdWatch ( aFd, AbstractEventSource::Read );
    }
    return true;
  }

  bool UnixSignalSource::removeSignal(int signum )
  {
    Z_D();
    if ( !d->_signalRefCount.count(signum) || d->_signalRefCount[signum] == 0 ) {
      return true;
    }
    d->_signalRefCount[signum]--;

    if ( d->_signalRefCount[signum] <= 0 ) {

      d->_signalRefCount.erase(signum);

      // remove the signal from our fd
      sigset_t     sigMask;
      sigemptyset ( &sigMask );
      for ( const auto &sig : d->_signalRefCount ) {
        sigaddset(&sigMask, sig.first);
      }

      auto res = signalfd ( d->_signalFd, &sigMask, SFD_NONBLOCK | SFD_CLOEXEC );
      if ( res == -1 ) {
        WAR << "Failed to update signalfd with errno: " << zypp::Errno() << std::endl;
        return false;
      }

      // unblock the signal
      sigemptyset ( &sigMask );
      sigaddset(&sigMask, signum);
      pthread_sigmask(SIG_UNBLOCK, &sigMask, NULL);
    }

    if ( d->_signalRefCount.size () == 0 ) {
      removeFdWatch ( d->_signalFd );
      d->_signalFd = -1;
    }
    return true;
  }

  SignalProxy<void (int)> UnixSignalSource::sigReceived()
  {
    return d_func()->_sigReceived;
  }

  void zyppng::UnixSignalSource::onFdReady( int fd, int events )
  {
    Z_D();
    struct signalfd_siginfo sfd_si;
    if ( read(fd, &sfd_si, sizeof(sfd_si)) == -1 ) {
      WAR << "Failed to read from signalfd" << std::endl;
      return;
    }

    if ( d->_signalRefCount.count ( sfd_si.ssi_signo ))
      d->_sigReceived.emit( sfd_si.ssi_signo );
    else
      WAR << "Received unexpected UNIX signal on signalFD: " << sfd_si.ssi_signo << std::endl;
  }

  void zyppng::UnixSignalSource::onSignal( int signal )
  {}

} // namespace zyppng
