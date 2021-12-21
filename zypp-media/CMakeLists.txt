# Collect all files that may contain translatable texts
FILE( GLOB_RECURSE POT_FILE_DEPENDS RELATIVE ${LIBZYPP_SOURCE_DIR} "*.h" "*.cc" )
SET( POT_FILE_DEPENDS_ZYPP_MEDIA ${POT_FILE_DEPENDS} PARENT_SCOPE )

INCLUDE_DIRECTORIES ( ${LIBZYPP_SOURCE_DIR} )

ADD_DEFINITIONS( -DLOCALEDIR="${CMAKE_INSTALL_PREFIX}/share/locale" -DTEXTDOMAIN="zypp" -DZYPP_DLL )

SET( zypp_media_HEADERS
  MediaConfig
  mediaconfig.h
  mediaexception.h
  MediaException
  mount.h
  Mount
)

SET( zypp_media_private_HEADERS
)

SET( zypp_media_SRCS
  mediaconfig.cc
  mediaexception.cc
  mount.cc
)

INSTALL(  FILES ${zypp_media_HEADERS} DESTINATION "${INCLUDE_INSTALL_DIR}/zypp-media" )

SET( zypp_media_auth_HEADERS
  auth/AuthData
  auth/authdata.h
  auth/CredentialFileReader
  auth/credentialfilereader.h
  auth/CredentialManager
  auth/credentialmanager.h
)

SET( zypp_media_auth_private_HEADERS
)

SET( zypp_media_auth_SRCS
  auth/authdata.cc
  auth/credentialfilereader.cc
  auth/credentialmanager.cc
)

INSTALL(  FILES ${zypp_media_auth_HEADERS} DESTINATION "${INCLUDE_INSTALL_DIR}/zypp-media/auth" )


SET( zypp_media_lib_SRCS
    ${zypp_media_SRCS}
    ${zypp_media_auth_SRCS}
)
SET( zypp_media_lib_HEADERS
    ${zypp_media_private_HEADERS} ${zypp_media_HEADERS}
    ${zypp_media_auth_private_HEADERS} ${zypp_media_auth_HEADERS}
)

# Default loggroup for all files
SET_LOGGROUP( "zypp-media" ${zypp_media_lib_SRCS} )

ADD_LIBRARY( zypp-media STATIC ${zypp_media_lib_SRCS} ${zypp_media_lib_HEADERS} )
#we include generated headers, so we need to wait for zypp-protobuf to be ready
add_dependencies( zypp-media zypp-protobuf )
