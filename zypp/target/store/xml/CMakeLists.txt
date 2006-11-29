INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )


SET(zypp_target_store_xml_SRCS
XMLPatchImpl.cc
XMLMessageImpl.cc
XMLScriptImpl.cc
XMLSelectionImpl.cc
XMLPatternImpl.cc
XMLProductImpl.cc
XMLAtomImpl.cc
)
ADD_LIBRARY(zypp_target_store_xml STATIC ${zypp_target_store_xml_SRCS})

SET_SOURCE_FILES_PROPERTIES(${zypp_target_store_xml_SRCS} PROPERTIES COMPILE_FLAGS -DZYPP_BASE_LOGGER_LOGGROUP=\\\"store_xml\\\")

INSTALL_FILES( FILES
XMLAtomImpl.h
XMLMessageImpl.h
XMLPatchImpl.h
XMLPatternImpl.h
XMLProductImpl.h
XMLScriptImpl.h
XMLSelectionImpl.h
DESTINATION
${CMAKE_INSTALL_PREFIX}/include/zypp/target/store/xml
)