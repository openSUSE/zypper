# Collect all files that may contain translatable texts
FILE( GLOB_RECURSE POT_FILE_DEPENDS RELATIVE ${LIBZYPP_SOURCE_DIR} "*.h" "*.cc" )
SET( POT_FILE_DEPENDS_ZYPP_CURL ${POT_FILE_DEPENDS} PARENT_SCOPE )

INCLUDE_DIRECTORIES ( ${LIBZYPP_SOURCE_DIR} )

ADD_DEFINITIONS( -DLOCALEDIR="${CMAKE_INSTALL_PREFIX}/share/locale" -DTEXTDOMAIN="zypp" -DZYPP_DLL )

SET( zypp_curl_HEADERS
  CurlConfig
  curlconfig.h
  ProxyInfo
  proxyinfo.h
  TransferSettings
  transfersettings.h
)

SET( zypp_curl_private_HEADERS
  private/curlhelper_p.h
)

SET( zypp_curl_SRCS
  curlconfig.cc
  proxyinfo.cc
  curlhelper.cc
  transfersettings.cc
)

INSTALL(  FILES ${zypp_curl_HEADERS} DESTINATION "${INCLUDE_INSTALL_DIR}/zypp-curl" )

SET( zypp_curl_auth_HEADERS
  auth/CurlAuthData
  auth/curlauthdata.h
)

SET( zypp_curl_auth_private_HEADERS
)

SET( zypp_curl_auth_SRCS
  auth/curlauthdata.cc
)

INSTALL(  FILES ${zypp_curl_auth_HEADERS} DESTINATION "${INCLUDE_INSTALL_DIR}/zypp-curl/auth" )

IF ( LIBPROXY_FOUND )
  SET(zypp_curl_proxyinfo_libproxy_SRCS proxyinfo/proxyinfolibproxy.cc)
  SET(zypp_curl_proxyinfo_libproxy_HEADERS proxyinfo/ProxyInfoLibproxy proxyinfo/proxyinfolibproxy.h)
ENDIF( LIBPROXY_FOUND )

SET( zypp_curl_proxyinfo_SRCS
  proxyinfo/proxyinfosysconfig.cc
  ${zypp_curl_proxyinfo_libproxy_SRCS}
)

SET( zypp_curl_proxyinfo_HEADERS
  proxyinfo/proxyinfoimpl.h
  proxyinfo/ProxyInfoSysconfig
  proxyinfo/proxyinfosysconfig.h
  ${zypp_curl_proxyinfo_libproxy_HEADERS}
  proxyinfo/proxyinfos.h
)

INSTALL(  FILES ${zypp_curl_proxyinfo_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR}/zypp-curl/proxyinfo )

SET( zypp_curl_ng_network_SRCS
  ng/network/downloader.cc
  ng/network/downloadspec.cc
  ng/network/mirrorcontrol.cc
  ng/network/networkrequestdispatcher.cc
  ng/network/networkrequesterror.cc
  ng/network/request.cc
)

SET( zypp_curl_ng_network_HEADERS
  ng/network/AuthData
  ng/network/authdata.h
  ng/network/Downloader
  ng/network/downloader.h
  ng/network/DownloadSpec
  ng/network/downloadspec.h
  ng/network/NetworkRequestDispatcher
  ng/network/networkrequestdispatcher.h
  ng/network/NetworkRequestError
  ng/network/networkrequesterror.h
  ng/network/Request
  ng/network/request.h
  ng/network/TransferSettings
  ng/network/transfersettings.h
)

SET( zypp_curl_ng_network_private_HEADERS
  ng/network/private/downloader_p.h
  ng/network/private/mediadebug_p.h
  ng/network/private/mirrorcontrol_p.h
  ng/network/private/networkrequestdispatcher_p.h
  ng/network/private/networkrequesterror_p.h
  ng/network/private/request_p.h
  ng/network/private/downloaderstates/base_p.h
  ng/network/private/downloaderstates/basicdownloader_p.h
  ng/network/private/downloaderstates/detectmeta_p.h
  ng/network/private/downloaderstates/final_p.h
  ng/network/private/downloaderstates/initial_p.h
  ng/network/private/downloaderstates/metalinkinfo_p.h
  ng/network/private/downloaderstates/metalink_p.h
  ng/network/private/downloaderstates/mirrorhandling_p.h
  ng/network/private/downloaderstates/normal_p.h
  ng/network/private/downloaderstates/preparemulti_p.h
  ng/network/private/downloaderstates/rangedownloader_p.h
  ng/network/private/downloaderstates/zck_p.h
)

SET( zypp_curl_ng_network_private_SRCS
  ng/network/private/downloaderstates/basicdownloader_p.cc
  ng/network/private/downloaderstates/detectmeta_p.cc
  ng/network/private/downloaderstates/final_p.cc
  ng/network/private/downloaderstates/initial_p.cc
  ng/network/private/downloaderstates/metalinkinfo_p.cc
  ng/network/private/downloaderstates/metalink_p.cc
  ng/network/private/downloaderstates/mirrorhandling_p.cc
  ng/network/private/downloaderstates/normal_p.cc
  ng/network/private/downloaderstates/preparemulti_p.cc
  ng/network/private/downloaderstates/rangedownloader_p.cc
  ng/network/private/downloaderstates/zck_p.cc
)

# ng files are not installed yet
# INSTALL(  FILES ${zypp_curl_ng_network_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR}/zypp-curl/ng/network )

SET( zypp_curl_parser_HEADERS
  parser/MediaBlockList
  parser/mediablocklist.h
  parser/MetaLinkParser
  parser/metalinkparser.h
)

SET( zypp_curl_parser_private_HEADERS
)

SET( zypp_curl_parser_SRCS
  parser/mediablocklist.cc
  parser/metalinkparser.cc
)

INSTALL(  FILES ${zypp_curl_parser_HEADERS} DESTINATION "${INCLUDE_INSTALL_DIR}/zypp-curl/parser" )


SET( zypp_curl_lib_SRCS
    ${zypp_curl_SRCS}
    ${zypp_curl_auth_SRCS}
    ${zypp_curl_ng_network_SRCS}
    ${zypp_curl_parser_SRCS}
    ${zypp_curl_proxyinfo_SRCS}
    ${zypp_curl_ng_network_private_SRCS}
)
SET( zypp_curl_lib_HEADERS
    ${zypp_curl_private_HEADERS} ${zypp_curl_HEADERS}
    ${zypp_curl_auth_private_HEADERS} ${zypp_curl_auth_HEADERS}
    ${zypp_curl_ng_network_HEADERS} ${zypp_curl_ng_network_private_HEADERS}
    ${zypp_curl_parser_private_HEADERS} ${zypp_curl_parser_HEADERS}
    ${zypp_curl_proxyinfo_HEADERS}
)

# Default loggroup for all files
SET_LOGGROUP( "zypp-curl" $${zypp_curl_lib_SRCS} )

ADD_LIBRARY( zypp-curl STATIC ${zypp_curl_lib_SRCS} ${zypp_curl_lib_HEADERS} )
#we include generated headers, so we need to wait for zypp-protobuf to be ready
add_dependencies( zypp-curl zypp-protobuf )

TARGET_LINK_LIBRARIES( zypp-curl ${CURL_LIBRARIES} )
TARGET_LINK_LIBRARIES( zypp-curl ${LIBPROXY_LIBRARIES} )
TARGET_LINK_LIBRARIES( zypp-curl ${LIBXML2_LIBRARIES} )
