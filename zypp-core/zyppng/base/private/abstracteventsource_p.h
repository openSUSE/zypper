/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
----------------------------------------------------------------------/
*
* This file contains private API, this might break at any time between releases.
* You have been warned!
*
*/
#ifndef ZYPPNG_BASE_PRIVATE_ABSTRACTEVENTSOURCE_P_H_INCLUDED
#define ZYPPNG_BASE_PRIVATE_ABSTRACTEVENTSOURCE_P_H_INCLUDED

#include "base_p.h"
#include <zypp-core/zyppng/base/eventdispatcher.h>

namespace zyppng {

class AbstractEventSourcePrivate : public BasePrivate
{
  ZYPP_DECLARE_PUBLIC(AbstractEventSource)

public:
  AbstractEventSourcePrivate( AbstractEventSource &p );
  std::weak_ptr<EventDispatcher> _ev;
};

}


#endif
