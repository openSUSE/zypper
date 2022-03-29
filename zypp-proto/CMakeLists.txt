SET( zypp_core_PROTOBUF_SOURCES
    core/envelope.proto
)

SET( zypp_media_PROTOBUF_SOURCES
  media/transfersettings.proto
  media/download.proto
  media/messages.proto
  media/networkrequesterror.proto
  media/provider.proto
)

SET( zypp_target_PROTOBUF_SOURCES
    target/commit.proto
)

SET( zypp_test_PROTOBUF_SOURCES
    test/tvm.proto
)

protobuf_generate_cpp( ZYPPCORE_PROTO_SRCS ZYPPCORE_PROTO_HDRS ${zypp_core_PROTOBUF_SOURCES} )
protobuf_generate_cpp( ZYPPMEDIA_PROTO_SRCS ZYPPMEDIA_PROTO_HDRS ${zypp_media_PROTOBUF_SOURCES} )
protobuf_generate_cpp( ZYPPTARGET_PROTO_SRCS ZYPPTARGET_PROTO_HDRS ${zypp_target_PROTOBUF_SOURCES} )
protobuf_generate_cpp( ZYPPTEST_PROTO_SRCS ZYPPTEST_PROTO_HDRS ${zypp_test_PROTOBUF_SOURCES} )

ADD_LIBRARY( zypp-protobuf STATIC
  ${ZYPPCORE_PROTO_SRCS} ${ZYPPCORE_PROTO_HDRS}
  ${ZYPPMEDIA_PROTO_SRCS} ${ZYPPMEDIA_PROTO_HDRS}
  ${ZYPPTARGET_PROTO_HDRS} ${ZYPPTARGET_PROTO_SRCS}
  ${ZYPPTEST_PROTO_HDRS} ${ZYPPTEST_PROTO_SRCS}
)
