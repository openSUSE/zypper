ADD_SUBDIRECTORY( schema ) 

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

SET(zypp_parser_yum_SRCS
YUMParserData.cc
YUMFileListParser.cc
YUMRepomdParser.cc
YUMGroupParser.cc
YUMPatternParser.cc
YUMPatchParser.cc
YUMPatchesParser.cc
YUMOtherParser.cc
YUMPrimaryParser.cc
YUMProductParser.cc
)
ADD_LIBRARY(zypp_parser_yum STATIC ${zypp_parser_yum_SRCS})

SET_SOURCE_FILES_PROPERTIES(${zypp_parser_yum_SRCS} PROPERTIES COMPILE_FLAGS -DZYPP_BASE_LOGGER_LOGGROUP=\\\"parser_yum\\\")

INSTALL_FILES( FILES
schemanames.h
YUMFileListParser.h
YUMGroupParser.h
YUMOtherParser.h
YUMParserData.h
YUMParser.h
YUMPatchesParser.h
YUMPatchParser.h
YUMPatternParser.h
YUMPrimaryParser.h
YUMProductParser.h
YUMRepomdParser.h
DESTINATION
${CMAKE_INSTALL_PREFIX}/include/zypp/parser/yum
)
