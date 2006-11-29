ADD_SUBDIRECTORY( schema ) 

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

SET(zypp_parser_xmlstore_SRCS
XMLParserData.cc
XMLPatternParser.cc
XMLLanguageParser.cc
XMLResObjectParser.cc
XMLPatchParser.cc
XMLProductParser.cc
XMLSourceCacheParser.cc
)
ADD_LIBRARY(zypp_parser_xmlstore STATIC ${zypp_parser_xmlstore_SRCS})

SET_SOURCE_FILES_PROPERTIES(${zypp_parser_xmlstore_SRCS} PROPERTIES COMPILE_FLAGS -DZYPP_BASE_LOGGER_LOGGROUP=\\\"parser_xmlstore\\\")

INSTALL_FILES( FILES
schemanames.h
XMLLanguageParser.h
XMLParserData.h
XMLPatchParser.h
XMLPatternParser.h
XMLProductParser.h
XMLResObjectParser.h
XMLSourceCacheParser.h
DESTINATION
${CMAKE_INSTALL_PREFIX}/include/zypp/parser/xmlstore
)
