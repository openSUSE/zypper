SET( zyppng_media_SRCS
  media/medianetwork.cc
  media/medianetworkserver.cc
  media/network/downloader.cc
  media/network/downloadspec.cc
  media/network/mirrorcontrol.cc
  media/network/networkrequestdispatcher.cc
  media/network/networkrequesterror.cc
  media/network/request.cc
)

SET( zyppng_media_HEADERS
  media/MediaNetwork
  media/medianetwork.h
  media/network/AuthData
  media/network/authdata.h
  media/network/downloader.h
  media/network/downloadspec.h
  media/network/networkrequestdispatcher.h
  media/network/networkrequesterror.h
  media/network/request.h
  media/network/transfersettings.h
  media/network/TransferSettings
)

SET( zyppng_media_private_HEADERS
  media/private/medianetworkserver_p.h
  media/network/private/downloader_p.h
  media/network/private/mediadebug_p.h
  media/network/private/mirrorcontrol_p.h
  media/network/private/networkrequestdispatcher_p.h
  media/network/private/networkrequesterror_p.h
  media/network/private/request_p.h
)

SET( zyppng_appcode_HEADERS
  Context
  context.h
)

SET( zyppng_appcode_SRCS
  context.cc
)

SET( zyppng_PROTOBUF_SOURCES
    proto/download.proto
    proto/messages.proto
    proto/networkrequesterror.proto
)

protobuf_generate_cpp(ZYPPNG_PROTO_SRCS ZYPPNG_PROTO_HDRS ${zyppng_PROTOBUF_SOURCES})


SET ( zyppng_HEADERS
  ${zyppng_appcode_HEADERS}
  ${zyppng_appcode_SRCS}
  ${zyppng_base_HEADERS}
  ${zyppng_base_private_HEADERS}
  ${zyppng_core_HEADERS}
  ${zyppng_media_HEADERS}
  ${zyppng_media_private_HEADERS}
  ${ZYPPNG_PROTO_HDRS}
)

SET ( zyppng_SRCS
  ${zyppng_media_SRCS}
  ${ZYPPNG_PROTO_SRCS}
)

ADD_LIBRARY( zyppng-objlib OBJECT ${zyppng_SRCS} ${zyppng_HEADERS} )

