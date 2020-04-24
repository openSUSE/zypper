/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\----------------------------------------------------------------------/
*
* This file contains private API, this might break at any time between releases.
* You have been warned!
*
*/

#include <zypp/zyppng/base/zyppglobal.h>
#include <zypp/zyppng/base/AbstractEventSource>
#include <zypp/zyppng/base/Signals>

namespace zyppng {
class SocketNotifierPrivate;

/*!
 * The SocketNotifier class provides a generic way to monitor activity on a file descriptors.
 *
 * Once a file descriptor was created using either low level OS API or was created by a 3rd party
 * library, the SocketNotifier can be created to integrate the file descriptor into the event loop.
 *
 * Each file descriptor can be monitored for all types of activiy supported by the base class \sa AbstractEventSource.
 *
 * \code
 * // Example code that uses a pipe() to communicate between a thread and the main loop
 * zyppng::EventLoop::Ptr loop = zyppng::EventLoop::create();
 *
 * int wakeupPipe[2] = { -1, -1 };
 * pipe2 ( wakeupPipe, O_NONBLOCK );
 *
 * //listen for activity on the event source
 * zyppng::SocketNotifier::Ptr notifier = zyppng::SocketNotifier::create( wakeupPipe[0], zyppng::SocketNotifier::Read, false );
 * notifier->sigActivated().connect([ &loop ]( const zyppng::SocketNotifier &notifier, int ) {
 *   char dummy;
 *   //read all available data
 *   while ( read( notifier.socket(), &dummy, 1 ) > 0 ) {
 *     std::cout<<dummy;
 *   }
 *   std::cout << std::endl;
 *   loop->quit();
 * });
 * notifier->setEnabled( true );
 *
 * std::thread t( [ &wakeupPipe ](){
 *   const char * test = "Some text";
 *   std::chrono::milliseconds dura( 1000 );
 *   std::this_thread::sleep_for( dura );
 *   write( wakeupPipe[1], test, 9 );
 *   std::this_thread::sleep_for( dura );
 *   return;
 * });
 *
 * loop->run();
 *
 * std::cout << "Bye Bye" << std::endl;
 * t.join();
 * \endcode
 */
class SocketNotifier : public AbstractEventSource
{
  ZYPP_DECLARE_PRIVATE( SocketNotifier )
public:

  using Ptr = std::shared_ptr<SocketNotifier>;
  using WeakPtr = std::weak_ptr<SocketNotifier>;

  using EventTypes = AbstractEventSource::EventTypes;

  /*!
   * Returns a new SocketNotifier
   * \param socket this is the filedescriptor to be modified
   * \param evTypes the event types that should be monitored \sa AbstractEventSource::EventTypes
   * \param enable If set to true the notifier is enabled right away, otherwise \sa setEnabled needs to be called explicitely
   */
  static Ptr create ( int socket, int evTypes, bool enable = true );

  /*!
   * Update the fd watch to the \a mode specified.
   * \sa AbstractEventSource::EventTypes
   */
  void setMode ( int mode );

  /*!
   * Returns the current mode used to monitor the file descriptor
   * \sa AbstractEventSource::EventTypes
   */
  int  mode () const;

  /*!
   * Enables or disables the SocketNotifier
   */
  void setEnabled ( bool enabled = true );

  /*!
   * Returns the monitored file descriptor
   */
  int socket () const;

  /*!
   * Emitted when there is activity on the socket according to the requested mode
   */
  SignalProxy<void (const SocketNotifier &sock, int evTypes)> sigActivated();

protected:
  SocketNotifier( int socket, int evTypes, bool enable  );

  // AbstractEventSource interface
public:
  void onFdReady(int, int events) override;
  void onSignal(int signal) override;
};

}
