INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

SET(zypp_source_plaindir_SRCS
PlaindirImpl.cc
)
ADD_LIBRARY(zypp_source_plaindir STATIC ${zypp_source_plaindir_SRCS})

SET_SOURCE_FILES_PROPERTIES(${zypp_source_plaindir_SRCS} PROPERTIES COMPILE_FLAGS -DZYPP_BASE_LOGGER_LOGGROUP=\\\"plaindir\\\")

INSTALL_FILES( FILES
PlaindirImpl.h
DESTINATION
${CMAKE_INSTALL_PREFIX}/include/zypp/source/plaindir
)
