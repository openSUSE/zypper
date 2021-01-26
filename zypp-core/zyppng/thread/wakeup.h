/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
----------------------------------------------------------------------*/
#ifndef ZYPP_NG_THREAD_WAKEUP_H_INCLUDED
#define ZYPP_NG_THREAD_WAKEUP_H_INCLUDED

#include <memory>

namespace zyppng {

  class SocketNotifier;

  /*!
 * Simple helper class that can be used to notify a thread to wake up.
 * This is guaranteed to use signal safe means to wake up the thread.
 */
  class Wakeup
  {
  public:
    Wakeup();
    ~Wakeup();

    /*!
     * Post a notification to the thread.
     */
    void notify();

    /*!
     * Called from the thread to acknowledge and clear the wakeup signal
     * \note If this is not called the EventLoop in the thread will constantly wake up
     */
    void ack();

    /*!
     * Returns the internal FD to be used in the thread to poll for notifications
     */
    int pollfd() const;

    /*!
     * Convenience function to create a \ref zyppng::SocketNotifier that is listening for incoming
     * notifications.
     */
    std::shared_ptr<SocketNotifier> makeNotifier( const bool enabled = true ) const;

  private:
    int _wakeupPipe[2] = { -1, -1 };
  };

}



#endif // ZYPP_NG_THREAD_WAKEUP_H_INCLUDED
