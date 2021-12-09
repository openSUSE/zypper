SET( zyppng_media_SRCS
  media/medianetwork.cc
  media/medianetworkserver.cc
)

SET( zyppng_media_HEADERS
  media/MediaNetwork
  media/medianetwork.h
)

SET( zyppng_media_private_HEADERS
  media/private/medianetworkserver_p.h
)

SET( zyppng_media_private_SOURCES
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
  ${zyppng_media_HEADERS}
  ${zyppng_media_private_HEADERS}
)

SET ( zyppng_SRCS
  ${zyppng_appcode_SRCS}
  ${zyppng_media_SRCS}
  ${zyppng_media_private_SOURCES}
)

ADD_LIBRARY( zyppng-objlib OBJECT ${zyppng_SRCS} ${zyppng_HEADERS} )
#we include generated headers, so we need to wait for zypp-protobuf to be ready
add_dependencies( zyppng-objlib zypp-protobuf )
