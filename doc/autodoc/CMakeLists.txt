### ##################################################

FIND_PROGRAM( DOT dot )
IF ( NOT DOT )
   MESSAGE( "dot not found: you should install graphviz." )
   SET( HAVE_DOT NO )
ELSE ( NOT DOT )
   SET( HAVE_DOT YES )
ENDIF ( NOT DOT )

SET( ZYPP_SOURCE_DIR  "${CMAKE_SOURCE_DIR}/zypp" )
SET( ZYPP_EXAMPLE_DIR "${CMAKE_SOURCE_DIR}/zypp/@DOXYGEN" )

SET( DOXYGEN_INPUT    "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile" )
SET( DOXYGEN_OUTPUT   "${CMAKE_CURRENT_BINARY_DIR}/html/index.html" )

### ##################################################

CONFIGURE_FILE(
   ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.cmake
   ${DOXYGEN_INPUT}
   @ONLY
   )

### ##################################################

ADD_CUSTOM_COMMAND (
   OUTPUT  ${DOXYGEN_OUTPUT}
   COMMAND ${CMAKE_COMMAND} -E echo_append "Building Documentation..."
   COMMAND ${DOXYGEN} ${DOXYGEN_INPUT}
   COMMAND ${CMAKE_COMMAND} -E echo "Done."
   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
   DEPENDS ${DOXYGEN_INPUT}
)

ADD_CUSTOM_TARGET( doc ALL
   DEPENDS ${DOXYGEN_OUTPUT}
)

ADD_CUSTOM_TARGET( doc_forced
   COMMAND ${CMAKE_COMMAND} -E echo_append "Building Documentation..."
   COMMAND ${DOXYGEN} ${DOXYGEN_INPUT}
   COMMAND ${CMAKE_COMMAND} -E echo "Done."
   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
   DEPENDS ${DOXYGEN_INPUT}
)

### ##################################################

INSTALL( FILES
   @PACKAGE@.doxytag
   DESTINATION ${DOC_INSTALL_DIR}
)
INSTALL( DIRECTORY
   html
   DESTINATION ${DOC_INSTALL_DIR}
)

