/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include "private/providemessage_p.h"

#include <zypp-core/Url.h>
#include <string_view>
#include <string>

namespace zyppng {

  static ProvideMessage::FieldVal fieldValFromProto ( const zypp::proto::DataField &field )
  {
    ProvideMessage::FieldVal v;
    switch ( field.field_val_case () ) {
      case zypp::proto::DataField::FieldValCase::kBoolVal:
        v = field.bool_val();
        break;
      case zypp::proto::DataField::FieldValCase::kDoubleVal:
        v = field.double_val();
        break;
      case zypp::proto::DataField::FieldValCase::kIntVal:
        v = field.int_val();
        break;
      case zypp::proto::DataField::FieldValCase::kLongVal:
        v = field.long_val();
        break;
      case zypp::proto::DataField::FieldValCase::kStrVal:
        v = field.str_val();
        break;
      case zypp::proto::DataField::FieldValCase::FIELD_VAL_NOT_SET:
        ZYPP_THROW( std::logic_error("Unexpected DataField type"));
        break;
    }
    return v;
  }

  static void fieldValToProto ( const ProvideMessage::FieldVal &val, zypp::proto::DataField &field )
  {
    if ( val.isString() )
      field.set_str_val( val.asString () );
    else if ( val.isInt() )
      field.set_int_val( val.asInt() );
    else if ( val.isInt64() )
      field.set_long_val( val.asInt64() );
    else if ( val.isDouble() )
      field.set_double_val( val.asDouble() );
    else if ( val.isBool() )
      field.set_bool_val( val.asBool() );
    else
      ZYPP_THROW( std::logic_error("Unexpected FieldVal type"));
  }

  static expected<void> validateMessage ( const ProvideMessage &msg )
  {
    const auto c = msg.code();
    const auto validCode =    ( c >= ProvideMessage::Code::FirstInformalCode    && c <= ProvideMessage::Code::LastInformalCode  )
                           || ( c >= ProvideMessage::Code::FirstSuccessCode     && c <= ProvideMessage::Code::LastSuccessCode   )
                           || ( c >= ProvideMessage::Code::FirstRedirCode       && c <= ProvideMessage::Code::LastRedirCode     )
                           || ( c >= ProvideMessage::Code::FirstClientErrCode   && c <= ProvideMessage::Code::LastClientErrCode )
                           || ( c >= ProvideMessage::Code::FirstSrvErrCode      && c <= ProvideMessage::Code::LastSrvErrCode    )
                           || ( c >= ProvideMessage::Code::FirstControllerCode  && c <= ProvideMessage::Code::LastControllerCode)
                           || ( c >= ProvideMessage::Code::FirstWorkerCode      && c <= ProvideMessage::Code::LastWorkerCode    );
    if ( !validCode ) {
      return zyppng::expected<void>::error( ZYPP_EXCPT_PTR ( InvalidMessageReceivedException("Invalid code in ProvideMessage")) );
    }

    #define DEF_REQ_FIELD( fname ) bool has_##fname = false

    #define REQ_FIELD_CHECK( msgtype, fname, ftype ) \
      if ( name == #fname ) { \
        if ( !std::holds_alternative<ftype>(val.asVariant()) ) { \
          error = ZYPP_EXCPT_PTR( InvalidMessageReceivedException( zypp::str::Str() << "Parse error " << #msgtype << ", Field " << #fname << " has invalid type" ) ); \
          return false; \
        } \
      has_##fname = true; \
      }

    #define OR_REQ_FIELD_CHECK( msgtype, fname, ftype ) else REQ_FIELD_CHECK( msgtype, fname, ftype )

    #define OPT_FIELD_CHECK( msgtype, fname, ftype ) \
      if ( name == #fname ) { \
        if ( !std::holds_alternative<ftype>(val.asVariant() ) ) { \
          error = ZYPP_EXCPT_PTR( InvalidMessageReceivedException( zypp::str::Str() << "Parse error " << #msgtype << ", Field " << #fname << " has invalid type" ) ); \
          return false; \
        } \
      }

    #define OR_OPT_FIELD_CHECK( msgtype, fname, ftype ) else OPT_FIELD_CHECK( msgtype, fname, ftype )

    #define FAIL_IF_NOT_SEEN_REQ_FIELD( msgtype, fname ) \
      if ( !has_##fname ) \
        return expected<void>::error( ZYPP_EXCPT_PTR( InvalidMessageReceivedException( zypp::str::Str() << #msgtype <<" message does not contain required " << #fname << " field" ) ) )

    #define FAIL_IF_ERROR( ) \
      if ( error ) return expected<void>::error( error )

    const auto &validateErrorMsg = []( const auto &msg ){
      std::exception_ptr error;
      DEF_REQ_FIELD(reason);
      msg.forEachVal( [&]( const auto &name, const ProvideMessage::FieldVal &val ){
        REQ_FIELD_CHECK ( Error, reason, std::string )
        OR_OPT_FIELD_CHECK ( Error, history, std::string )
        OR_OPT_FIELD_CHECK ( Error, transient, bool )
        return true;
      });
      FAIL_IF_NOT_SEEN_REQ_FIELD( Error, reason );
      FAIL_IF_ERROR();
      return expected<void>::success();
    };

    switch ( c )
    {
      case ProvideMessage::Code::ProvideStarted: {
        std::exception_ptr error;
        DEF_REQ_FIELD(url);
        msg.forEachVal( [&]( const auto &name, const ProvideMessage::FieldVal &val ){
          REQ_FIELD_CHECK ( ProvideStarted, url, std::string )
          OR_OPT_FIELD_CHECK ( ProvideStarted, local_filename, std::string )
          OR_OPT_FIELD_CHECK ( ProvideStarted, staging_filename, std::string )
          return true;
        });
        FAIL_IF_NOT_SEEN_REQ_FIELD( ProvideStarted, url );
        FAIL_IF_ERROR();
        break;
      }
      case ProvideMessage::Code::ProvideFinished: {
        std::exception_ptr error;
        DEF_REQ_FIELD(cacheHit);
        DEF_REQ_FIELD(local_filename);
        msg.forEachVal( [&]( const auto &name, const ProvideMessage::FieldVal &val ){
          REQ_FIELD_CHECK     ( ProvideFinished, cacheHit,       bool )
          OR_REQ_FIELD_CHECK  ( ProvideFinished, local_filename, std::string )
          return true;
        });
        FAIL_IF_NOT_SEEN_REQ_FIELD( ProvideFinished, cacheHit );
        FAIL_IF_NOT_SEEN_REQ_FIELD( ProvideFinished, local_filename );
        FAIL_IF_ERROR();
        break;
      }
      case ProvideMessage::Code::AttachFinished: {
        // no fields
        break;
      }
      case ProvideMessage::Code::DetachFinished: {
        // no fields
        break;
      }
      case ProvideMessage::Code::AuthInfo: {
        std::exception_ptr error;
        DEF_REQ_FIELD(username);
        DEF_REQ_FIELD(password);
        DEF_REQ_FIELD(auth_timestamp);
        msg.forEachVal( [&]( const auto &name, const ProvideMessage::FieldVal &val ){
          REQ_FIELD_CHECK    ( AuthInfo, username, std::string )
          OR_REQ_FIELD_CHECK ( AuthInfo, password, std::string )
          OR_REQ_FIELD_CHECK ( AuthInfo, auth_timestamp, int64_t )
          OR_OPT_FIELD_CHECK ( AuthInfo, authType, std::string )
          return true;
        });
        FAIL_IF_NOT_SEEN_REQ_FIELD( ProvideStarted, username );
        FAIL_IF_NOT_SEEN_REQ_FIELD( ProvideStarted, password );
        FAIL_IF_NOT_SEEN_REQ_FIELD( ProvideStarted, auth_timestamp );
        FAIL_IF_ERROR();
        break;
      }
      case ProvideMessage::Code::MediaChanged:
        /* No Fields */
          break;
      case ProvideMessage::Code::Redirect: {
        std::exception_ptr error;
        DEF_REQ_FIELD(new_url);
        msg.forEachVal( [&]( const auto &name, const ProvideMessage::FieldVal &val ){
          REQ_FIELD_CHECK ( Redirect, new_url, std::string )
          return true;
        });
        FAIL_IF_NOT_SEEN_REQ_FIELD( Redirect, new_url );
        FAIL_IF_ERROR();
        break;
      }
      case ProvideMessage::Code::Metalink: {
        std::exception_ptr error;
        DEF_REQ_FIELD(new_url);
        msg.forEachVal( [&]( const auto &name, const ProvideMessage::FieldVal &val ){
          REQ_FIELD_CHECK ( Metalink, new_url, std::string )
          return true;
        });
        FAIL_IF_NOT_SEEN_REQ_FIELD( Metalink, new_url );
        FAIL_IF_ERROR();
        break;
      }
      case ProvideMessage::Code::BadRequest:
      case ProvideMessage::Code::Unauthorized:
      case ProvideMessage::Code::Forbidden:
      case ProvideMessage::Code::PeerCertificateInvalid:
      case ProvideMessage::Code::NotFound:
      case ProvideMessage::Code::ExpectedSizeExceeded:
      case ProvideMessage::Code::ConnectionFailed:
      case ProvideMessage::Code::Timeout:
      case ProvideMessage::Code::Cancelled:
      case ProvideMessage::Code::InvalidChecksum:
      case ProvideMessage::Code::MountFailed:
      case ProvideMessage::Code::Jammed:
      case ProvideMessage::Code::NoAuthData:
      case ProvideMessage::Code::MediaChangeAbort:
      case ProvideMessage::Code::MediaChangeSkip:
      case ProvideMessage::Code::InternalError: {
        const auto &e = validateErrorMsg(msg);
        if ( !e )
          return e;
        break;
      }
      case ProvideMessage::Code::Provide: {
        std::exception_ptr error;
        DEF_REQ_FIELD(url);
        msg.forEachVal( [&]( const auto &name, const ProvideMessage::FieldVal &val ){
          REQ_FIELD_CHECK    ( Provide, url, std::string )
          OR_OPT_FIELD_CHECK ( Provide, filename, std::string )
          OR_OPT_FIELD_CHECK ( Provide, delta_file, std::string )
          OR_OPT_FIELD_CHECK ( Provide, expected_filesize, int64_t )
          OR_OPT_FIELD_CHECK ( Provide, check_existance_only, bool )
          OR_OPT_FIELD_CHECK ( Provide, metalink_enabled, bool )
          return true;
        });
        FAIL_IF_NOT_SEEN_REQ_FIELD( Provide, url );
        FAIL_IF_ERROR();
        break;
      }
      case ProvideMessage::Code::Cancel:
        /* No Fields */
          break;

      case ProvideMessage::Code::Attach: {
        std::exception_ptr error;

        DEF_REQ_FIELD(url);
        DEF_REQ_FIELD(attach_id);
        DEF_REQ_FIELD(label);

        // not really required, but this way we can check if all false or all true
        DEF_REQ_FIELD(verify_type);
        DEF_REQ_FIELD(verify_data);
        DEF_REQ_FIELD(media_nr);

        msg.forEachVal( [&]( const auto &name, const ProvideMessage::FieldVal &val ){
          REQ_FIELD_CHECK    ( Attach, url        , std::string )
          OR_REQ_FIELD_CHECK ( Attach, attach_id  , std::string )
          OR_REQ_FIELD_CHECK ( Attach, label      , std::string )
          OR_REQ_FIELD_CHECK ( Attach, verify_type, std::string )
          OR_REQ_FIELD_CHECK ( Attach, verify_data, std::string )
          OR_REQ_FIELD_CHECK ( Attach, media_nr   , int32_t )
          OR_OPT_FIELD_CHECK ( Attach, device     , std::string )
          return true;
        });
        FAIL_IF_NOT_SEEN_REQ_FIELD( Provide, url );
        FAIL_IF_NOT_SEEN_REQ_FIELD( Provide, label );
        FAIL_IF_NOT_SEEN_REQ_FIELD( Provide, attach_id );
        if ( ! ( ( has_verify_data == has_verify_type ) && ( has_verify_type == has_media_nr ) ) )
          return expected<void>::error( ZYPP_EXCPT_PTR ( InvalidMessageReceivedException("Error in Attach message, one of the following fields is not set or invalid: ( verify_type, verify_data, media_nr ). Either none or all need to be set. ")) );
        FAIL_IF_ERROR();
        break;
      }
      case ProvideMessage::Code::Detach: {
        std::exception_ptr error;
        DEF_REQ_FIELD(url);
        msg.forEachVal( [&]( const auto &name, const ProvideMessage::FieldVal &val ){
          REQ_FIELD_CHECK ( Detach, url, std::string )
          return true;
        });
        FAIL_IF_NOT_SEEN_REQ_FIELD( Detach, url );
        FAIL_IF_ERROR();
        break;
      }
      case ProvideMessage::Code::AuthDataRequest: {
        std::exception_ptr error;
        DEF_REQ_FIELD(effective_url);
        msg.forEachVal( [&]( const auto &name, const ProvideMessage::FieldVal &val ){
          REQ_FIELD_CHECK     ( AuthDataRequest, effective_url,       std::string )
          OR_OPT_FIELD_CHECK  ( AuthDataRequest, last_auth_timestamp, int64_t     )
          OR_OPT_FIELD_CHECK  ( AuthDataRequest, username,            std::string )
          OR_OPT_FIELD_CHECK  ( AuthDataRequest, authHint,            std::string )
          return true;
        });
        FAIL_IF_NOT_SEEN_REQ_FIELD( AuthDataRequest, effective_url );
        FAIL_IF_ERROR();
        break;
      }
      case ProvideMessage::Code::MediaChangeRequest: {
        std::exception_ptr error;
        DEF_REQ_FIELD(label);
        DEF_REQ_FIELD(media_nr);
        DEF_REQ_FIELD(device);
        msg.forEachVal( [&]( const auto &name, const ProvideMessage::FieldVal &val ){
          REQ_FIELD_CHECK    ( MediaChangeRequest, label,     std::string )
          OR_REQ_FIELD_CHECK ( MediaChangeRequest, media_nr,  int32_t     )
          OR_REQ_FIELD_CHECK ( MediaChangeRequest, device,    std::string )
          OR_OPT_FIELD_CHECK ( MediaChangeRequest, desc,      std::string )
          return true;
        });
        FAIL_IF_NOT_SEEN_REQ_FIELD( MediaChangeRequest, label );
        FAIL_IF_NOT_SEEN_REQ_FIELD( MediaChangeRequest, media_nr );
        FAIL_IF_NOT_SEEN_REQ_FIELD( MediaChangeRequest, device );
        FAIL_IF_ERROR();
        break;
      }
      default: {
        // all error messages have the same format
        if ( c >= ProvideMessage::Code::FirstClientErrCode && c <= ProvideMessage::Code::LastSrvErrCode ) {
          const auto &e = validateErrorMsg(msg);
          if ( !e )
            return e;
        }
        break;
      }
    }
    return expected<void>::success();
  }

  ProvideMessage::ProvideMessage()
    : _impl ( new zypp::proto::ProvideMessage )
  { }

  expected<zyppng::ProvideMessage> ProvideMessage::create(const RpcMessage &message)
  {
    ProvideMessage msg;
    const auto &res = RpcMessageStream::parseMessageInto<zypp::proto::ProvideMessage>( message, *msg._impl );
    if ( res ) {
      const auto &valid = validateMessage(msg);
      if ( !valid ) {
        ERR << "Invalid message for ID: " << msg._impl->request_id() << std::endl;;
        return zyppng::expected<zyppng::ProvideMessage>::error( valid.error() );
      }

      return zyppng::expected<zyppng::ProvideMessage>::success( std::move(msg) );
    }
    ERR << "Failed to parse message" << std::endl;;
    return zyppng::expected<zyppng::ProvideMessage>::error( res.error() );
  }

  expected<ProvideMessage> ProvideMessage::create( const zypp::proto::ProvideMessage &message )
  {
    ProvideMessage msg;
    *msg._impl = std::move(message);
    const auto &valid = validateMessage(msg);
    if ( !valid ) {
      ERR << "Invalid message for ID: " << msg._impl->request_id() << std::endl;;
      return zyppng::expected<zyppng::ProvideMessage>::error( valid.error() );
    }

    return zyppng::expected<zyppng::ProvideMessage>::success( std::move(msg) );
  }

  ProvideMessage ProvideMessage::createProvideStarted( const uint32_t reqId, const zypp::Url &url, const std::optional<std::string> &localFilename, const std::optional<std::string> &stagingFilename )
  {
    ProvideMessage msg;
    msg.setCode ( ProvideMessage::Code::ProvideStarted );
    msg.setRequestId ( reqId );
    msg.setValue ( ProvideStartedMsgFields::Url, url.asCompleteString() );
    if ( localFilename )
      msg.setValue ( ProvideStartedMsgFields::LocalFilename, *localFilename );
    if ( stagingFilename )
      msg.setValue ( ProvideStartedMsgFields::StagingFilename, *stagingFilename );

    return msg;
  }

  ProvideMessage ProvideMessage::createProvideFinished( const uint32_t reqId, const std::string &localFilename, bool cacheHit )
  {
    ProvideMessage msg;
    msg.setCode ( ProvideMessage::Code::ProvideFinished );
    msg.setRequestId ( reqId );
    msg.setValue ( ProvideFinishedMsgFields::LocalFilename, localFilename );
    msg.setValue ( ProvideFinishedMsgFields::CacheHit, cacheHit );

    return msg;
  }

  ProvideMessage ProvideMessage::createAttachFinished( const uint32_t reqId )
  {
    ProvideMessage msg;
    msg.setCode ( ProvideMessage::Code::AttachFinished );
    msg.setRequestId ( reqId );

    return msg;
  }

  ProvideMessage ProvideMessage::createDetachFinished(const uint32_t reqId)
  {
    ProvideMessage msg;
    msg.setCode ( ProvideMessage::Code::DetachFinished );
    msg.setRequestId ( reqId );

    return msg;
  }

  ProvideMessage ProvideMessage::createAuthInfo( const uint32_t reqId, const std::string &user, const std::string &pw, int64_t timestamp, const std::map<std::string, std::string> &extraValues )
  {
    ProvideMessage msg;
    msg.setCode ( ProvideMessage::Code::AuthInfo );
    msg.setRequestId ( reqId );
    msg.setValue ( AuthInfoMsgFields::Username, user  );
    msg.setValue ( AuthInfoMsgFields::Password, pw    );
    msg.setValue ( AuthInfoMsgFields::AuthTimestamp, timestamp );
    for ( auto i : extraValues ) {
      msg.setValue( i.first, i.second );
    }
    return msg;
  }

  ProvideMessage ProvideMessage::createMediaChanged( const uint32_t reqId )
  {
    ProvideMessage msg;
    msg.setCode ( ProvideMessage::Code::MediaChanged );
    msg.setRequestId ( reqId );

    return msg;
  }

  ProvideMessage ProvideMessage::createRedirect( const uint32_t reqId, const zypp::Url &newUrl )
  {
    ProvideMessage msg;
    msg.setCode ( ProvideMessage::Code::Redirect );
    msg.setRequestId ( reqId );
    msg.setValue ( RedirectMsgFields::NewUrl, newUrl.asCompleteString() );

    return msg;
  }

  ProvideMessage ProvideMessage::createMetalinkRedir( const uint32_t reqId, const std::vector<zypp::Url> &newUrls )
  {
    ProvideMessage msg;
    msg.setCode ( ProvideMessage::Code::Metalink );
    msg.setRequestId ( reqId );
    for( const auto &val : newUrls )
      msg.addValue( MetalinkRedirectMsgFields::NewUrl, val.asCompleteString() );

    return msg;
  }

  ProvideMessage ProvideMessage::createErrorResponse( const uint32_t reqId, const uint code, const std::string &reason, bool transient )
  {
    ProvideMessage msg;
    if ( code < Code::FirstClientErrCode || code > Code::LastSrvErrCode )
      ZYPP_THROW(std::out_of_range("code must be between 400 and 599"));
    msg.setCode ( code );
    msg.setRequestId ( reqId );
    msg.setValue ( ErrMsgFields::Reason, reason );
    msg.setValue ( ErrMsgFields::Transient, transient );
    return msg;
  }

  ProvideMessage ProvideMessage::createProvide( const uint32_t reqId, const zypp::Url &url, const std::optional<std::string> &filename, const std::optional<std::string> &deltaFile, const std::optional<int64_t> &expFilesize, bool checkExistOnly )
  {
    ProvideMessage msg;
    msg.setCode ( ProvideMessage::Code::Provide );
    msg.setRequestId ( reqId );
    msg.setValue ( ProvideMsgFields::Url, url.asCompleteString() );

    if ( filename )
      msg.setValue ( ProvideMsgFields::Filename, *filename );
    if ( deltaFile )
      msg.setValue ( ProvideMsgFields::DeltaFile, *deltaFile );
    if ( expFilesize )
      msg.setValue ( ProvideMsgFields::ExpectedFilesize, *expFilesize );
    msg.setValue ( ProvideMsgFields::CheckExistOnly, checkExistOnly );

    return msg;
  }

  ProvideMessage ProvideMessage::createCancel( const uint32_t reqId )
  {
    ProvideMessage msg;
    msg.setCode ( ProvideMessage::Code::Cancel );
    msg.setRequestId ( reqId );

    return msg;
  }

  ProvideMessage ProvideMessage::createAttach(const uint32_t reqId, const zypp::Url &url, const std::string attachId, const std::string &label, const std::optional<std::string> &verifyType, const std::optional<std::string> &verifyData, const std::optional<int32_t> &mediaNr )
  {
    ProvideMessage msg;
    msg.setCode ( ProvideMessage::Code::Attach );
    msg.setRequestId ( reqId );
    msg.setValue ( AttachMsgFields::Url, url.asCompleteString() );
    msg.setValue ( AttachMsgFields::AttachId, attachId );
    msg.setValue ( AttachMsgFields::Label, label );

    if ( verifyType.has_value() && verifyData.has_value() && mediaNr.has_value() ) {
      msg.setValue ( AttachMsgFields::VerifyType, *verifyType );
      msg.setValue ( AttachMsgFields::VerifyData, *verifyData );
      msg.setValue ( AttachMsgFields::MediaNr, *mediaNr );
    } else {
      if ( !( ( verifyType.has_value() == verifyData.has_value() ) && ( verifyData.has_value() == mediaNr.has_value() ) ) )
        WAR << "Attach message requires verifyType, verifyData and mediaNr either set together or not set at all." << std::endl;
    }

    return msg;
  }

  ProvideMessage ProvideMessage::createDetach( const uint32_t reqId, const zypp::Url &attachUrl )
  {
    ProvideMessage msg;
    msg.setCode ( ProvideMessage::Code::Detach );
    msg.setRequestId ( reqId );
    msg.setValue ( DetachMsgFields::Url, attachUrl.asCompleteString() );

    return msg;
  }

  ProvideMessage ProvideMessage::createAuthDataRequest( const uint32_t reqId, const zypp::Url &effectiveUrl, const std::string &lastTriedUser, const std::optional<int64_t> &lastAuthTimestamp, const std::map<std::string, std::string> &extraValues )
  {
    ProvideMessage msg;
    msg.setCode ( ProvideMessage::Code::AuthDataRequest );
    msg.setRequestId ( reqId );
    msg.setValue ( AuthDataRequestMsgFields::EffectiveUrl, effectiveUrl.asCompleteString() );
    if ( lastTriedUser.size() )
      msg.setValue( AuthDataRequestMsgFields::LastUser, lastTriedUser );
    if ( lastAuthTimestamp )
      msg.setValue ( AuthDataRequestMsgFields::LastAuthTimestamp, *lastAuthTimestamp );

    return msg;
  }

  ProvideMessage ProvideMessage::createMediaChangeRequest( const uint32_t reqId, const std::string &label, int32_t mediaNr, const std::vector<std::string> &devices, const std::optional<std::string> &desc )
  {
    ProvideMessage msg;
    msg.setCode ( ProvideMessage::Code::MediaChangeRequest );
    msg.setRequestId ( reqId );
    msg.setValue ( MediaChangeRequestMsgFields::Label, label );
    msg.setValue ( MediaChangeRequestMsgFields::MediaNr, mediaNr );
    for ( const auto &device : devices )
      msg.addValue ( MediaChangeRequestMsgFields::Device, device );
    if ( desc )
      msg.setValue ( MediaChangeRequestMsgFields::Desc, *desc );

    return msg;
  }

  uint ProvideMessage::requestId() const
  {
    return _impl->request_id();
  }

  void ProvideMessage::setRequestId(const uint id)
  {
    _impl->set_request_id( id );
  }

  uint ProvideMessage::code() const
  {
    return _impl->message_code();
  }

  void ProvideMessage::setCode( const uint newCode )
  {
    _impl->set_message_code ( newCode );
  }

  std::vector<ProvideMessage::FieldVal> ProvideMessage::values( const std::string_view &str ) const
  {
    std::vector<ProvideMessage::FieldVal> values;
    const auto &fields = _impl->fields();
    for ( const auto &field : fields ) {
      if ( field.key() != str )
        continue;
      values.push_back( fieldValFromProto(field) );
    }
    return values;
  }

  std::vector<ProvideMessage::FieldVal> ProvideMessage::values( const std::string &str ) const
  {
    return values( std::string_view(str));
  }

  ProvideMessage::FieldVal ProvideMessage::value ( const std::string_view &str, const FieldVal &defaultVal ) const
  {
    const auto &fields = _impl->fields();
    auto i = std::find_if( fields.rbegin(), fields.rend(), [&str]( const auto &val ){ return val.key() == str; } );
    if ( i == fields.rend() )
      return defaultVal;
    return fieldValFromProto(*i);
  }

  ProvideMessage::FieldVal ProvideMessage::value( const std::string &str, const FieldVal &defaultVal ) const
  {
    return value( std::string_view(str), defaultVal );
  }

  void ProvideMessage::setValue( const std::string &name, const FieldVal &value )
  {
    setValue( std::string_view(name), value );
  }

  void ProvideMessage::setValue( const std::string_view &name, const FieldVal &value )
  {
    auto &fields = *_impl->mutable_fields();
    auto i = std::find_if( fields.rbegin(), fields.rend(), [&name]( const auto &val ){ return val.key() == name; } );
    if ( i == fields.rend() ) {
      auto &newVal = *_impl->add_fields();
      newVal.set_key( name.data() );
      fieldValToProto( value, newVal );
    } else
      fieldValToProto( value, *i );
  }

  void ProvideMessage::addValue( const std::string &name, const FieldVal &value )
  {
    return addValue( std::string_view(name), value );
  }

  void ProvideMessage::addValue( const std::string_view &name, const FieldVal &value )
  {
    auto &newVal = *_impl->add_fields();
    newVal.set_key( name.data() );
    fieldValToProto( value, newVal );
  }

  void ProvideMessage::forEachVal( const std::function<bool (const std::string &, const FieldVal &)> &cb ) const
  {
    auto &fields = _impl->fields();
    for ( const auto &val : fields ) {
      if ( !cb( val.key(), fieldValFromProto(val) ) ) {
        return;
      }
    }
  }

  zypp::proto::ProvideMessage &ProvideMessage::impl()
  {
    return *_impl.get();
  }

  const zypp::proto::ProvideMessage &ProvideMessage::impl() const
  {
    return *_impl.get();
  }

}
