#ifndef ZYPPNG_UNIXSIGNALSOURCE_H
#define ZYPPNG_UNIXSIGNALSOURCE_H

#include "abstracteventsource.h"

namespace zyppng {

  ZYPP_FWD_DECL_TYPE_WITH_REFS ( UnixSignalSource );
  ZYPP_FWD_DECL_TYPE_WITH_REFS ( EventDispatcher );
  class UnixSignalSourcePrivate;

  class UnixSignalSource : public AbstractEventSource
  {
    ZYPP_DECLARE_PRIVATE (UnixSignalSource);
  public:
    ~UnixSignalSource() override;

    bool addSignal( int signum );
    bool removeSignal( int signum );

    SignalProxy<void(int signum)> sigReceived();

  protected:
    // AbstractEventSource interface
    void onFdReady(int fd, int events) override;
    void onSignal(int signal) override;

  private:
    friend class EventDispatcher;
    static UnixSignalSourceRef create ();
    UnixSignalSource();

  };
} // namespace zyppng

#endif // ZYPPNG_UNIXSIGNALSOURCE_H
