/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
#ifndef ZYPP_MEDIA_PROVIDE_H_INCLUDED
#define ZYPP_MEDIA_PROVIDE_H_INCLUDED

#include <zypp-core/ManagedFile.h>
#include <zypp-core/zyppng/base/zyppglobal.h>
#include <zypp-core/zyppng/base/Base>
#include <zypp-core/zyppng/async/AsyncOp>
#include <zypp-core/zyppng/pipelines/expected.h>
#include <zypp-core/ByteCount.h>
#include <zypp-media/ng/ProvideFwd>
#include <zypp-media/ng/ProvideRes>
#include <zypp-media/auth/AuthData>
#include <boost/any.hpp>

namespace zypp {
  class Url;
  namespace media {
    struct CredManagerOptions;
  }
}

/*!
 * @TODO Fix bsc#1174011 "auth=basic ignored in some cases" for provider
 *       We should proactively add the password to the request if basic auth is configured
 *       and a password is available in the credentials but not in the URL.
 *
 *       We should be a bit paranoid here and require that the URL has a user embedded, otherwise we go the default route
 *       and ask the server first about the auth method
 */
namespace zyppng {

  class ProvidePrivate;
  using AnyMap = std::unordered_map<std::string, boost::any>;

  /*!
  * RAII helper for media handles
  */
  class ProvideMediaHandle
  {
    public:
      ProvideMediaHandle () = default;
      ProvideMediaHandle ( Provide &parent, const std::string &hdl );
      std::shared_ptr<Provide> parent() const;
      bool isValid () const;
      std::string handle() const;
    private:
      struct Data;
      std::shared_ptr<Data> _ref;
  };

  /*!
   * Provide status observer object, this can be used to provide good insight into the status of the provider, its items and
   * all running requests.
   */
  class ProvideStatus
  {
    public:

      struct Stats {
        std::chrono::steady_clock::time_point _startTime;
        std::chrono::steady_clock::time_point _lastPulseTime;
        uint            _itemsSinceStart = 0; //< How many items have been started since Provide::start() was called
        uint            _runningItems    = 0; //< How many items are currently running
        zypp::ByteCount _finishedBytes; //< The number of bytes that were finished completely
        zypp::ByteCount _expectedBytes; //< The number of currently expected bytes
        zypp::ByteCount _partialBytes;  //< The number of bytes of items that were already partially downloaded but the item they belong to is not finished
        zypp::ByteCount _perSecondSinceLastPulse; //< The download speed since the last pulse
        zypp::ByteCount _perSecond;               //< The download speed we are currently operating with
      };

      ProvideStatus( ProvideRef parent );
      virtual ~ProvideStatus(){}

      virtual void provideStart ();
      virtual void provideDone  (){}
      virtual void itemStart    ( ProvideItem &item ){}
      virtual void itemDone     ( ProvideItem &item );
      virtual void itemFailed   ( ProvideItem &item );
      virtual void requestStart    ( ProvideItem &item, uint32_t reqId, const zypp::Url &url, const AnyMap &extraData = {} ){}
      virtual void requestDone     ( ProvideItem &item, uint32_t reqId, const AnyMap &extraData = {} ){}
      virtual void requestRedirect ( ProvideItem &item, uint32_t reqId, const zypp::Url &toUrl, const AnyMap &extraData = {} ){}
      virtual void requestFailed   ( ProvideItem &item, uint32_t reqId, const std::exception_ptr err, const AnyMap &requestData = {} ){}
      virtual void pulse ( );

      const Stats &stats() const;

    private:
      Stats _stats;
      ProvideWeakRef _provider;
  };

  class Provide : public Base
  {
    ZYPP_DECLARE_PRIVATE(Provide);
    template<class T> friend class ProvidePromise;
    friend class ProvideItem;
    friend class ProvideMediaHandle;
    friend class ProvideStatus;
  public:

    using MediaHandle = ProvideMediaHandle;

    static ProvideRef create( const zypp::Pathname &workDir = "" );

    AsyncOpRef<expected<MediaHandle>> attachMedia( const std::vector<zypp::Url> &urls, const ProvideMediaSpec &request );
    AsyncOpRef<expected<MediaHandle>> attachMedia( const zypp::Url &url, const ProvideMediaSpec &request );

    AsyncOpRef<expected<ProvideRes>> provide(  const std::vector<zypp::Url> &urls, const ProvideFileSpec &request );
    AsyncOpRef<expected<ProvideRes>> provide(  const zypp::Url &url, const ProvideFileSpec &request );
    AsyncOpRef<expected<ProvideRes>> provide(  const MediaHandle &attachHandle, const zypp::Pathname &fileName, const ProvideFileSpec &request );

    void start();
    void setWorkerPath( const zypp::Pathname &path );
    bool isRunning() const;
    bool ejectDevice ( const std::string &queueRef, const std::string &device );

    void setStatusTracker( ProvideStatusRef tracker );

    const zypp::Pathname &providerWorkdir () const;

    const zypp::media::CredManagerOptions &credManangerOptions () const;
    void setCredManagerOptions( const zypp::media::CredManagerOptions & opt );

    SignalProxy<void()> sigIdle();

    enum Action {
      ABORT,  // abort and return error
      RETRY,  // retry
      SKIP    // abort and set skip request
    };
    using MediaChangeAction = std::optional<Action>;

    /*!
     * Connect to this signal to handle media change requests
     *
     * \note It is NOT supported to shutdown the provider or cancel items when in this callback
     *       Returning Abort here will effectively cancel the current item anyway.
     */
    SignalProxy<MediaChangeAction( const std::string &queueRef, const std::string &label, const int32_t mediaNr, const std::vector<std::string> &devices, const std::optional<std::string> &desc )> sigMediaChangeRequested( );

    /*!
     * This signal is emitted in case a request signaled a need to get Auth Info and nothing was found
     * in the \ref zypp::media::CredentialManager.
     */
    SignalProxy< std::optional<zypp::media::AuthData> ( const zypp::Url &reqUrl, const std::string &triedUsername, const std::map<std::string, std::string> &extraValues ) > sigAuthRequired();

  private:
    Provide(  const zypp::Pathname &workDir );
    void releaseMedia( const std::string &mediaRef );
  };

}
#endif
