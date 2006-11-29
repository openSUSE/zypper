INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )


########### next target ###############

SET(zypp_media_proxyinfo_SRCS
ProxyInfoSysconfig.cc
)

ADD_LIBRARY(zypp_media_proxyinfo STATIC ${zypp_media_proxyinfo_SRCS})

SET_SOURCE_FILES_PROPERTIES(${zypp_media_proxyinfo_SRCS} PROPERTIES COMPILE_FLAGS -DZYPP_BASE_LOGGER_LOGGROUP=\\\"meda-proxyinfo\\\")

INSTALL_FILES( FILES
ProxyInfoImpl.h
ProxyInfos.h
ProxyInfoSysconfig.h
DESTINATION
${CMAKE_INSTALL_PREFIX}/include/zypp/media/proxyinfo
)