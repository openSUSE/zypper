# Collect all files that may contain translatable texts
FILE( GLOB_RECURSE POT_FILE_DEPENDS RELATIVE ${LIBZYPP_SOURCE_DIR} "*.h" "*.cc" )
SET( POT_FILE_DEPENDS_ZYPP_MEDIA ${POT_FILE_DEPENDS} PARENT_SCOPE )

INCLUDE_DIRECTORIES ( ${LIBZYPP_SOURCE_DIR} )

ADD_DEFINITIONS( -DLOCALEDIR="${CMAKE_INSTALL_PREFIX}/share/locale" -DTEXTDOMAIN="zypp" -DZYPP_DLL )

SET( zypp_media_HEADERS
  CDTools
  cdtools.h
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
  cdtools.cc
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

SET( zypp_media_ng_HEADERS
  ng/headervaluemap.h
  ng/HeaderValueMap
  ng/provide.h
  ng/Provide
  ng/providefwd.h
  ng/ProvideFwd
  ng/provide-configvars.h
  ng/providespec.h
  ng/ProvideSpec
  ng/provideres.h
  ng/provideitem.h
  ng/ProvideRes
  ng/mediaverifier.h
  ng/MediaVerifier
  ng/worker/provideworker.h
  ng/worker/ProvideWorker
  ng/worker/mountingworker.h
  ng/worker/MountingWorker
)

SET( zypp_media_ng_private_HEADERS
  ng/private/attachedmediainfo_p.h
  ng/private/provide_p.h
  ng/private/providefwd_p.h
  ng/private/provideitem_p.h
  ng/private/providemessage_p.h
  ng/private/providequeue_p.h
  ng/private/providedbg_p.h

)

SET( zypp_media_ng_SRCS
  ng/headervaluemap.cc
  ng/provide.cc
  ng/provideres.cc
  ng/providespec.cc
  ng/provideitem.cc
  ng/providemessage.cc
  ng/providequeue.cc
  ng/mediaverifier.cc
  ng/worker/provideworker.cc
  ng/worker/mountingworker.cc
)

INSTALL(  FILES ${zypp_media_ng_HEADERS} DESTINATION "${INCLUDE_INSTALL_DIR}/zypp-media/ng" )


SET( zypp_media_lib_SRCS
    ${zypp_media_SRCS}
    ${zypp_media_auth_SRCS}
    ${zypp_media_ng_SRCS}
)
SET( zypp_media_lib_HEADERS
    ${zypp_media_private_HEADERS} ${zypp_media_HEADERS}
    ${zypp_media_auth_private_HEADERS} ${zypp_media_auth_HEADERS}
    ${zypp_media_ng_private_HEADERS} ${zypp_media_ng_HEADERS}
)

# Default loggroup for all files
SET_LOGGROUP( "zypp-media" ${zypp_media_lib_SRCS} )

ADD_LIBRARY( zypp-media STATIC ${zypp_media_lib_SRCS} ${zypp_media_lib_HEADERS} )
#we include generated headers, so we need to wait for zypp-protobuf to be ready
add_dependencies( zypp-media zypp-protobuf )
