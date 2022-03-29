/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
#ifndef ZYPP_MEDIA_PROVIDE_CONFIGVARS_H_INCLUDED
#define ZYPP_MEDIA_PROVIDE_CONFIGVARS_H_INCLUDED

#include <string_view>

namespace zyppng {
  // special config strings sent to the workers:
  constexpr std::string_view AGENT_STRING_CONF("zconfig://media/UserAgent");
  constexpr std::string_view DISTRO_FLAV_CONF("zconfig://media/DistributionFlavor");
  constexpr std::string_view ANON_ID_CONF("zconfig://media/AnonymousId");
  constexpr std::string_view ATTACH_POINT("zconfig://media/AttachPoint");
  constexpr std::string_view PROVIDER_ROOT("zconfig://media/ProviderRoot");


  // request related settings:
  constexpr std::string_view NETWORK_METALINK_ENABLED("zypp-nw-metalink-enabled");  //< Enable or disable metalink for a specific request
  constexpr std::string_view HANDLER_SPECIFIC_DEVICES("zypp-req-specific-devices"); //< Limit the request to a set of devices. Devices are comma seperated.
}

#endif
