SET( zyppng_base_SRCS
  base/abstracteventsource.cc
  base/base.cc
  base/eventdispatcher_glib.cc
  base/eventloop_glib.cc
  base/linuxhelpers.cc
  base/timer.cc
  base/threaddata.cc
  base/socketnotifier.cc
)

SET( zyppng_base_HEADERS
  base/AbstractEventSource
  base/abstracteventsource.h
  base/AutoDisconnect
  base/autodisconnect.h
  base/Base
  base/base.h
  base/EventDispatcher
  base/eventdispatcher.h
  base/EventLoop
  base/eventloop.h
  base/signals.h
  base/SocketNotifier
  base/socketnotifier.h
  base/statemachine.h
  base/Timer
  base/timer.h
  base/zyppglobal.h
)

SET( zyppng_base_private_HEADERS
  base/private/abstracteventsource_p.h
  base/private/base_p.h
  base/private/eventdispatcher_glib_p.h
  base/private/eventloop_glib_p.h
  base/private/linuxhelpers_p.h
  base/private/threaddata_p.h
)

SET( zyppng_core_HEADERS
  core/bytearray.h
  core/ByteArray
  core/url.h
  core/Url
  core/idstring.h
  core/IdString
  core/string.h
  core/String
)

SET( zyppng_io_private_HEADERS
  io/private/iobuffer_p.h
  io/private/iodevice_p.h
  io/private/socket_p.h
  io/private/sockaddr_p.h
)

SET( zyppng_io_SRCS
  io/iobuffer.cc
  io/iodevice.cc
  io/socket.cc
  io/sockaddr.cpp
  )

SET( zyppng_io_HEADERS
  io/IODevice
  io/iodevice.h
  io/Socket
  io/socket.h
  io/sockaddr.h
  io/SockAddr
)


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

SET( zyppng_thread_SRCS
  thread/asyncqueue.cc
  thread/wakeup.cpp
)

SET( zyppng_thread_HEADERS
  thread/AsyncQueue
  thread/asyncqueue.h
  thread/private/asyncqueue_p.h
  thread/Wakeup
  thread/wakeup.h
)

SET( zyppng_thread_private_HEADERS
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
  ${zyppng_io_HEADERS}
  ${zyppng_io_private_HEADERS}
  ${zyppng_media_HEADERS}
  ${zyppng_media_private_HEADERS}
  ${zyppng_thread_HEADERS}
  ${zyppng_thread_private_HEADERS}
  ${ZYPPNG_PROTO_HDRS}
)

SET ( zyppng_SRCS
  ${zyppng_base_SRCS}
  ${zyppng_io_SRCS}
  ${zyppng_media_SRCS}
  ${zyppng_thread_SRCS}
  ${ZYPPNG_PROTO_SRCS}
)

ADD_LIBRARY( zyppng-objlib OBJECT ${zyppng_SRCS} ${zyppng_HEADERS} )

