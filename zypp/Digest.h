/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/Digest.h
 *
 * \todo replace by Blocxx
 *
*/

#ifndef ZYPP_MEDIA_DIGEST_H
#define ZYPP_MEDIA_DIGEST_H

#include <zypp-core/Digest.h>

#include <zypp/Callback.h>
#include <zypp/Pathname.h>

namespace zypp {

  struct DigestReport : public callback::ReportBase
  {
    virtual bool askUserToAcceptNoDigest( const zypp::Pathname &file );
    virtual bool askUserToAccepUnknownDigest( const Pathname &file, const std::string &name );
    virtual bool askUserToAcceptWrongDigest( const Pathname &file, const std::string &requested, const std::string &found );
  };

} // namespace zypp
#endif
