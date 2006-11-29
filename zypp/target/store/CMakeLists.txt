ADD_SUBDIRECTORY( xml ) 

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

SET(zypp_target_store_SRCS
PersistentStorage.cc
Backend.cc
XMLFilesBackend.cc
serialize.cc
xml_escape_parser.cpp
)

ADD_LIBRARY(zypp_target_store STATIC ${zypp_target_store_SRCS})
TARGET_LINK_LIBRARIES(zypp_target_store zypp_target_store_xml)

SET_SOURCE_FILES_PROPERTIES(${zypp_target_store_SRCS} PROPERTIES COMPILE_FLAGS -DZYPP_BASE_LOGGER_LOGGROUP=\\\"target_store\\\")

INSTALL_FILES( FILES
Backend.h
PersistentStorage.h
serialize.h
XMLFilesBackend.h
DESTINATION
${CMAKE_INSTALL_PREFIX}/include/zypp/target/store
)