/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
----------------------------------------------------------------------*/
#ifndef ZYPP_NG_CORE_CONTEXT_H_INCLUDED
#define ZYPP_NG_CORE_CONTEXT_H_INCLUDED

#include <memory>

namespace zyppng {

  class EventLoop;
  class MirrorControl;

  class Context {

  public:

    using Ptr = std::shared_ptr<Context>;

    Context();
    std::shared_ptr<EventLoop> evLoop () const;
    std::shared_ptr<MirrorControl> mirrorControl ();

  private:
    std::shared_ptr<EventLoop> _zyppEventLoop;
    std::shared_ptr<MirrorControl> _mirrorControl;

  };

}

#endif
