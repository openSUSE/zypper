SET( zyppng_base_SRCS
  base/abstracteventsource.cc
  base/base.cc
  base/eventdispatcher_glib.cc
  base/eventloop_glib.cc
  base/timer.cc
  base/threaddata.cc
  base/socketnotifier.cc
)

SET( zyppng_base_HEADERS
  base/AbstractEventSource
  base/abstracteventsource.h
  base/Base
  base/base.h
  base/EventDispatcher
  base/eventdispatcher.h
  base/EventLoop
  base/eventloop.h
  base/signals.h
  base/SocketNotifier
  base/socketnotifier.h
  base/Timer
  base/timer.h
  base/zyppglobal.h
)

SET( zyppng_base_private_HEADERS
  base/private/abstracteventsource_p.h
  base/private/base_p.h
  base/private/eventdispatcher_glib_p.h
  base/private/eventloop_glib_p.h
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
)

SET( zyppng_io_SRCS
  io/iobuffer.cc
  )

SET( zyppng_io_HEADERS
)


SET( zyppng_media_SRCS
  media/medianetwork.cc
  media/network/downloader.cc
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
  media/network/networkrequestdispatcher.h
  media/network/networkrequesterror.h
  media/network/request.h
  media/network/transfersettings.h
  media/network/TransferSettings
)

SET( zyppng_media_private_HEADERS
  media/network/private/downloader_p.h
  media/network/private/mirrorcontrol_p.h
  media/network/private/networkrequestdispatcher_p.h
  media/network/private/networkrequesterror_p.h
  media/network/private/request_p.h
)

SET( zyppng_thread_SRCS
  thread/asyncqueue.cc
)

SET( zyppng_thread_HEADERS
  thread/AsyncQueue
  thread/asyncqueue.h
  thread/private/asyncqueue_p.h
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
)

SET ( zyppng_SRCS
  ${zyppng_base_SRCS}
  ${zyppng_io_SRCS}
  ${zyppng_media_SRCS}
  ${zyppng_thread_SRCS}
)

ADD_LIBRARY( zyppng-objlib OBJECT ${zyppng_SRCS} ${zyppng_HEADERS} )

