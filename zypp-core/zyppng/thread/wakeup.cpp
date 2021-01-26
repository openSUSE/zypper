#include "wakeup.h"

#include <zypp-core/zyppng/base/SocketNotifier>

#include <unistd.h>
#include <fcntl.h>

namespace zyppng {

  Wakeup::Wakeup()
  {
    ::pipe ( _wakeupPipe );
    ::fcntl( _wakeupPipe[0], F_SETFL, O_NONBLOCK );
  }

  Wakeup::~Wakeup()
  {
    ::close (_wakeupPipe[0]);
    ::close (_wakeupPipe[1]);
  }

  void Wakeup::notify()
  {
    write( _wakeupPipe[1], "\n", 1);
  }

  void Wakeup::ack()
  {
    char dummy;
    while ( ::read( _wakeupPipe[0], &dummy, 1 ) > 0 ) { continue; }
  }

  int Wakeup::pollfd() const
  {
    return _wakeupPipe[0];
  }

  std::shared_ptr<SocketNotifier> Wakeup::makeNotifier( const bool enabled ) const
  {
    return SocketNotifier::create( _wakeupPipe[0], zyppng::SocketNotifier::Read, enabled );
  }

}
