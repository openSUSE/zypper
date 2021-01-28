SET( zyppng_PROTOBUF_SOURCES

)

protobuf_generate_cpp(ZYPPNG_PROTO_SRCS ZYPPNG_PROTO_HDRS ${zyppng_PROTOBUF_SOURCES})

ADD_LIBRARY( zyppng-protobuf OBJECT ${ZYPPNG_PROTO_SRCS} ${ZYPPNG_PROTO_HDRS} )

add_dependencies( zyppng-protobuf zypp-protobuf )
