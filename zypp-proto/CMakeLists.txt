ADD_LIBRARY( zypp-protobuf STATIC
  core/envelope.proto media/provider.proto target/commit.proto test/tvm.proto
)

protobuf_generate( TARGET zypp-protobuf )

target_include_directories ( zypp-protobuf PRIVATE ${CMAKE_CURRENT_BINARY_DIR} )
