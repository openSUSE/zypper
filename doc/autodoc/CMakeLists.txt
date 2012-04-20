### ##################################################
IF ( DOXYGEN )
### ##################################################

FIND_PROGRAM( DOT dot )
IF ( NOT DOT )
   MESSAGE( "dot not found: you should install graphviz." )
   SET( HAVE_DOT NO )
ELSE ( NOT DOT )
   SET( HAVE_DOT YES )
ENDIF ( NOT DOT )

SET( ZYPP_SOURCE_DIR     ${LIBZYPP_SOURCE_DIR}/zypp )
SET( ZYPP_DOCINCLUDE_DIR ${LIBZYPP_SOURCE_DIR}/doc/autoinclude )
SET( ZYPP_EXAMPLE_DIR    ${LIBZYPP_SOURCE_DIR}/examples )

SET( DOXYGEN_INPUT    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile )
SET( DOXYGEN_OUTPUT   ${CMAKE_CURRENT_BINARY_DIR}/html/index.html )
FILE( MAKE_DIRECTORY  ${CMAKE_CURRENT_BINARY_DIR}/html )

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
   COMMAND ${DOXYGEN} ${DOXYGEN_INPUT} || { echo '*** IGNORE FAILED DOXYGEN - DOC MIGHT BE INCOMPLETE!'\; true\; }
   COMMAND ${CMAKE_COMMAND} -E echo "Done."
   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
   DEPENDS ${DOXYGEN_INPUT}
)

ADD_CUSTOM_TARGET( doc ALL
   DEPENDS ${DOXYGEN_OUTPUT}
)

ADD_CUSTOM_TARGET( doc_forced
   COMMAND ${CMAKE_COMMAND} -E echo_append "Building Documentation..."
   COMMAND ${DOXYGEN} ${DOXYGEN_INPUT} || { echo '*** IGNORE FAILED DOXYGEN - DOC MIGHT BE INCOMPLETE!'\; true\; }
   COMMAND ${CMAKE_COMMAND} -E echo "Done."
   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
   DEPENDS ${DOXYGEN_INPUT}
)

### ##################################################

INSTALL( FILES
   ${CMAKE_CURRENT_BINARY_DIR}/libzypp.doxytag
   DESTINATION ${DOC_INSTALL_DIR}/libzypp/libzypp.doxytag
)

# We could use the thing below but it wont work with cmake older than 2.4.4
FILE(GLOB docfiles ${CMAKE_CURRENT_BINARY_DIR}/html/*)
INSTALL( FILES
   ${docfiles}
   DESTINATION ${DOC_INSTALL_DIR}/libzypp/html
)

#INSTALL( DIRECTORY
#   ${CMAKE_CURRENT_BINARY_DIR}/html
#   DESTINATION ${DOC_INSTALL_DIR}
#)
### ##################################################
ENDIF ( DOXYGEN )
### ##################################################

