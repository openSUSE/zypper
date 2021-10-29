# Collect all files that may contain translatable texts
FILE( GLOB_RECURSE POT_FILE_DEPENDS RELATIVE ${LIBZYPP_SOURCE_DIR} "*.h" "*.cc" )
SET( POT_FILE_DEPENDS_ZYPP_MEDIA ${POT_FILE_DEPENDS} PARENT_SCOPE )

INCLUDE_DIRECTORIES ( ${LIBZYPP_SOURCE_DIR} )

ADD_DEFINITIONS( -DLOCALEDIR="${CMAKE_INSTALL_PREFIX}/share/locale" -DTEXTDOMAIN="zypp" -DZYPP_DLL )

SET( zypp_media_HEADERS
  mediaexception.h
  MediaException
  mount.h
  Mount
)

SET( zypp_media_SRCS
  mediaexception.cc
  mount.cc
)

INSTALL(  FILES ${zypp_media_HEADERS} DESTINATION "${INCLUDE_INSTALL_DIR}/zypp-media" )

ADD_LIBRARY( zypp-media STATIC ${zypp_media_SRCS} ${zypp_media_HEADERS} )
#we include generated headers, so we need to wait for zypp-protobuf to be ready
add_dependencies( zypp-media zypp-protobuf )
