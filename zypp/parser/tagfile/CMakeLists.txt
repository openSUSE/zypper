INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

SET(zypp_parser_tagfile_SRCS
ParseException.cc
TagFileParser.cc
)

ADD_LIBRARY(zypp_parser_tagfile STATIC ${zypp_parser_tagfile_SRCS})

SET_SOURCE_FILES_PROPERTIES(${zypp_parser_tagfile_SRCS} PROPERTIES COMPILE_FLAGS -DZYPP_BASE_LOGGER_LOGGROUP=\\\"parser-tagfile\\\")

INSTALL_FILES( FILES
ParseException.h
TagFileParser.h
DESTINATION
${CMAKE_INSTALL_PREFIX}/include/zypp/parser/tagfile
)
