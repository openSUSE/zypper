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
*
*/
#ifndef ZYPP_MEDIA_PRIVATE_PROVIDE_MESSAGE_P_H_INCLUDED
#define ZYPP_MEDIA_PRIVATE_PROVIDE_MESSAGE_P_H_INCLUDED

#include <zypp-core/base/PtrTypes.h>
#include <zypp-core/zyppng/pipelines/Expected>
#include <zypp-core/zyppng/rpc/MessageStream>
#include <zypp-media/ng/ProvideSpec> // for FieldType
#include <zypp-media/ng/HeaderValueMap>
#include <variant>
#include <functional>
#include <zypp-proto/provider.pb.h>

namespace zypp::proto {
  class ProvideMessage;
  class Envelope;
}

namespace zyppng {

  namespace ProvideStartedMsgFields
  {
    constexpr std::string_view Url ("url");
    constexpr std::string_view LocalFilename ("local_filename");
    constexpr std::string_view StagingFilename ("staging_filename");
  }

  namespace ProvideFinishedMsgFields
  {
    constexpr std::string_view LocalFilename ("local_filename");
    constexpr std::string_view CacheHit ("cacheHit");
  }

  namespace AuthInfoMsgFields
  {
    constexpr std::string_view Username ("username");
    constexpr std::string_view Password ("password");
    constexpr std::string_view AuthTimestamp ("auth_timestamp");
    constexpr std::string_view AuthType ("authType");
  }

  namespace RedirectMsgFields
  {
    constexpr std::string_view NewUrl ("new_url");
  }

  namespace MetalinkRedirectMsgFields
  {
    constexpr std::string_view NewUrl ("new_url");
  }

  namespace ErrMsgFields
  {
    constexpr std::string_view Reason ("reason");
    constexpr std::string_view Transient ("transient");
    constexpr std::string_view History ("history");
  }

  namespace ProvideMsgFields
  {
    constexpr std::string_view Url ("url");
    constexpr std::string_view Filename ("filename");
    constexpr std::string_view DeltaFile ("delta_file");
    constexpr std::string_view ExpectedFilesize ("expected_filesize");
    constexpr std::string_view CheckExistOnly ("check_existance_only");
    constexpr std::string_view MetalinkEnabled ("metalink_enabled");
  }

  namespace AttachMsgFields
  {
    constexpr std::string_view Url ("url");
    constexpr std::string_view AttachId ("attach_id");
    constexpr std::string_view VerifyType ("verify_type");
    constexpr std::string_view VerifyData ("verify_data");
    constexpr std::string_view MediaNr ("media_nr");
    constexpr std::string_view Device  ("device");
    constexpr std::string_view Label   ("label");
  }

  namespace DetachMsgFields
  {
    constexpr std::string_view Url ("url");
  }

  namespace AuthDataRequestMsgFields
  {
    constexpr std::string_view EffectiveUrl ("effective_url");
    constexpr std::string_view LastAuthTimestamp ("last_auth_timestamp");
    constexpr std::string_view LastUser ("username");
    constexpr std::string_view AuthHint ("authHint");
  }

  namespace MediaChangeRequestMsgFields
  {
    constexpr std::string_view Label ("label");
    constexpr std::string_view MediaNr ("media_nr");
    constexpr std::string_view Device ("device");
    constexpr std::string_view Desc ("desc");
  }

  namespace EjectMsgFields
  {
    constexpr std::string_view device ("device");
  }

  class ProvideMessage
  {
  public:
    using Code = zypp::proto::MessageCodes;
    using FieldVal = HeaderValue;

    static expected<ProvideMessage> create ( const zyppng::RpcMessage &message );
    static expected<ProvideMessage> create ( const zypp::proto::ProvideMessage &message );
    static ProvideMessage createProvideStarted  ( const uint32_t reqId, const zypp::Url &url , const std::optional<std::string> &localFilename = {}, const std::optional<std::string> &stagingFilename = {} );
    static ProvideMessage createProvideFinished ( const uint32_t reqId, const std::string &localFilename , bool cacheHit );
    static ProvideMessage createAttachFinished  ( const uint32_t reqId );
    static ProvideMessage createDetachFinished  ( const uint32_t reqId );
    static ProvideMessage createAuthInfo ( const uint32_t reqId, const std::string &user, const std::string &pw, int64_t timestamp, const std::map<std::string, std::string> &extraValues = {} );
    static ProvideMessage createMediaChanged ( const uint32_t reqId );
    static ProvideMessage createRedirect ( const uint32_t reqId, const zypp::Url &newUrl );
    static ProvideMessage createMetalinkRedir ( const uint32_t reqId, const std::vector<zypp::Url> &newUrls );
    static ProvideMessage createErrorResponse ( const uint32_t reqId, const uint code, const std::string &reason, bool transient = false  );

    static ProvideMessage createProvide         ( const uint32_t reqId
                                                  , const zypp::Url &url
                                                  , const std::optional<std::string> &filename = {}
                                                  , const std::optional<std::string> &deltaFile = {}
                                                  , const std::optional<int64_t> &expFilesize = {}
                                                  , bool checkExistOnly = false );

    static ProvideMessage createCancel          ( const uint32_t reqId );

    static ProvideMessage createAttach( const uint32_t reqId
                                      , const zypp::Url &url
                                      , const std::string attachId
                                      , const std::string &label
                                      , const std::optional<std::string> &verifyType = {}
                                      , const std::optional<std::string> &verifyData = {}
                                      , const std::optional<int32_t> &mediaNr = {} );

    static ProvideMessage createDetach              ( const uint32_t reqId, const zypp::Url &attachUrl );
    static ProvideMessage createAuthDataRequest     ( const uint32_t reqId, const zypp::Url &effectiveUrl, const std::string &lastTriedUser ="", const std::optional<int64_t> &lastAuthTimestamp = {}, const std::map<std::string, std::string> &extraValues = {} );
    static ProvideMessage createMediaChangeRequest  ( const uint32_t reqId, const std::string &label, int32_t mediaNr, const std::vector<std::string> &devices, const std::optional<std::string> &desc );

    uint requestId () const;
    void setRequestId ( const uint id );

    uint code () const;
    void setCode ( const uint newCode );

    std::vector<FieldVal> values ( const std::string_view &str ) const;
    std::vector<FieldVal> values ( const std::string &str ) const;
    HeaderValueMap headers() const;
    /*!
     * Returns the last entry with key \a str in the list of values
     * or the default value specified in \a defaultVal
     */
    FieldVal value ( const std::string_view &str, const FieldVal &defaultVal = FieldVal() ) const;
    FieldVal value ( const std::string &str, const FieldVal &defaultVal = FieldVal() ) const;
    void setValue  ( const std::string &name, const FieldVal &value );
    void setValue  ( const std::string_view &name, const FieldVal &value );
    void addValue  ( const std::string &name, const FieldVal &value );
    void addValue  ( const std::string_view &name, const FieldVal &value );
    void forEachVal( const std::function<bool( const std::string &name, const FieldVal &val)> &cb  ) const;

    zypp::proto::ProvideMessage &impl();
    const zypp::proto::ProvideMessage &impl() const;

  private:
    ProvideMessage();
    zypp::RWCOW_pointer<zypp::proto::ProvideMessage> _impl;
  };
}

namespace zypp {
  template<>
  inline zypp::proto::ProvideMessage* rwcowClone<zypp::proto::ProvideMessage>( const zypp::proto::ProvideMessage * rhs )
  { return new zypp::proto::ProvideMessage(*rhs); }
}



#endif
