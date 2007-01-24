#ADD_SUBDIRECTORY( yum2 ) 
ADD_SUBDIRECTORY( yum ) 
ADD_SUBDIRECTORY( tagfile ) 
ADD_SUBDIRECTORY( xmlstore ) 

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

SET(zypp_parser_SRCS
XMLNodeIterator.cc
LibXMLHelper.cc
SAXParser.cc
)

ADD_LIBRARY(zypp_parser STATIC ${zypp_parser_SRCS})
TARGET_LINK_LIBRARIES(zypp_parser zypp_parser_yum)
TARGET_LINK_LIBRARIES(zypp_parser zypp_parser_xmlstore)
TARGET_LINK_LIBRARIES(zypp_parser zypp_parser_tagfile)

SET_SOURCE_FILES_PROPERTIES(${zypp_parser_SRCS} PROPERTIES COMPILE_FLAGS -DZYPP_BASE_LOGGER_LOGGROUP=\\\"parser\\\")

INSTALL_FILES( FILES
LibXMLHelper.h
ParserProgress.h
SAXParser.h
XMLNodeIterator.h
xml_parser_assert.h
DESTINATION
${CMAKE_INSTALL_PREFIX}/include/zypp/parser
)
