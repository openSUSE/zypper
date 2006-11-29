INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

SET(zypp_target_modalias_SRCS
Modalias.cc
)
ADD_LIBRARY(zypp_target_modalias STATIC ${zypp_target_modalias_SRCS})

SET_SOURCE_FILES_PROPERTIES(${zypp_target_modalias_SRCS} PROPERTIES COMPILE_FLAGS -DZYPP_BASE_LOGGER_LOGGROUP=\\\"modalias\\\")

INSTALL_FILES( FILES
Modalias.h
DESTINATION
${CMAKE_INSTALL_PREFIX}/include/zypp/target/modalias
)
