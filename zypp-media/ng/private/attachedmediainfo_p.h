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
#ifndef ZYPP_MEDIA_PRIVATE_ATTACHEDMEDIAINFO_P_H_INCLUDED
#define ZYPP_MEDIA_PRIVATE_ATTACHEDMEDIAINFO_P_H_INCLUDED

#include "providefwd_p.h"
#include "providequeue_p.h"
#include <zypp-media/ng/ProvideSpec>
#include <string>
#include <chrono>

namespace zyppng {

  class ProvidePrivate;

  struct AttachedMediaInfo {

  public:
    void ref() {
      if ( _refCount == 0 )
        _idleSince = std::chrono::steady_clock::time_point::max();

      _refCount++;
    }
    void unref() {
      if ( _refCount > 0 ) {
        _refCount--;

        if ( _refCount == 0 )
          _idleSince = std::chrono::steady_clock::now();
      }
    }

    /*!
     * Returns true if \a other requests the same medium as this instance
     */
    bool isSameMedium ( const std::vector<zypp::Url> &urls, const ProvideMediaSpec &spec ) {

      const auto check = _spec.isSameMedium(spec);
      if ( !zypp::indeterminate (check) )
        return (bool)check;

      // let the URL rule
      return ( std::find( urls.begin(), urls.end(), _attachedUrl ) != urls.end() );
    }

    std::string _name;
    ProvideQueueWeakRef _backingQueue; //< if initialized contains a weak reference to the queue that owns this medium
    ProvideQueue::Config::WorkerType _workerType;
    zypp::Url   _attachedUrl; // the URL that was used for the attach request
    ProvideMediaSpec _spec;
    uint _refCount = 0;
    std::chrono::steady_clock::time_point _idleSince = std::chrono::steady_clock::time_point::max();
  };

}

#endif
