ADD_SUBDIRECTORY( proxyinfo ) 

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )


########### next target ###############

SET(zypp_media_SRCS
MediaException.cc
MediaAccess.cc
MediaHandler.cc
Mount.cc
MediaNFS.cc
MediaCD.cc
MediaDIR.cc
MediaDISK.cc
MediaSMB.cc
MediaCIFS.cc
ProxyInfo.cc
MediaCurl.cc
MediaISO.cc
MediaManager.cc
)
ADD_LIBRARY(zypp_media STATIC ${zypp_media_SRCS})
TARGET_LINK_LIBRARIES(zypp_media zypp_media_proxyinfo)

SET_SOURCE_FILES_PROPERTIES(${zypp_media_SRCS} PROPERTIES COMPILE_FLAGS -DZYPP_BASE_LOGGER_LOGGROUP=\\\"media\\\")

INSTALL_FILES( FILES
MediaAccess.h
MediaCD.h
MediaCIFS.h
MediaCurl.h
MediaDIR.h
MediaDISK.h
MediaException.h
MediaHandler.h
MediaISO.h
MediaManager.h
MediaNFS.h
MediaSMB.h
MediaSource.h
Mount.h
ProxyInfo.h
DESTINATION
${CMAKE_INSTALL_PREFIX}/include/zypp/media
)