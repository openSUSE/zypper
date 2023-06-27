/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
----------------------------------------------------------------------*/
#ifndef ZYPP_NG_RPC_RPC_H_INCLUDED
#define ZYPP_NG_RPC_RPC_H_INCLUDED

#include <zypp-proto/core/envelope.pb.h>

namespace zyppng::rpc {
  /*!
     Type used as header before each zypp::proto::Envelope
   */
  using HeaderSizeType = uint32_t;
}

#endif
