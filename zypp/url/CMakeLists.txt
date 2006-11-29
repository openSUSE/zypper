#INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )


########### next target ###############

SET(zypp_url_SRCS
UrlUtils.cc
UrlBase.cc
)
ADD_LIBRARY(zypp_url STATIC ${zypp_url_SRCS})

SET_SOURCE_FILES_PROPERTIES(${zypp_url_SRCS} PROPERTIES COMPILE_FLAGS -DZYPP_BASE_LOGGER_LOGGROUP=\\\"url\\\")

INSTALL_FILES( FILES
UrlBase.h
UrlException.h
UrlUtils.h
DESTINATION
${CMAKE_INSTALL_PREFIX}/include/zypp/url
)
