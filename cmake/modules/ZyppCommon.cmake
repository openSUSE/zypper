# Library
IF ( DEFINED LIB )
  SET ( LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/${LIB}" )
ELSE ( DEFINED  LIB )
  IF (CMAKE_SIZEOF_VOID_P MATCHES "8")
    SET( LIB_SUFFIX "64" )
  ENDIF(CMAKE_SIZEOF_VOID_P MATCHES "8")
  SET ( LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}" )
ENDIF ( DEFINED  LIB )
MESSAGE(STATUS "Libraries will be installed in ${LIB_INSTALL_DIR}" )

# system configuration dir (etc)
IF( NOT DEFINED SYSCONFDIR )
  IF ( ${CMAKE_INSTALL_PREFIX} STREQUAL "/usr" )
    # if installing in usr, set sysconfg to etc
    SET( SYSCONFDIR /etc )
  ELSE ( ${CMAKE_INSTALL_PREFIX} STREQUAL "/usr" )
    SET ( SYSCONFDIR "${CMAKE_INSTALL_PREFIX}/etc" )
  ENDIF ( ${CMAKE_INSTALL_PREFIX} STREQUAL "/usr" )
ENDIF( NOT DEFINED SYSCONFDIR )
MESSAGE(STATUS "Config files will be installed in ${SYSCONFDIR}" )

# usr INSTALL_PREFIX

IF( DEFINED CMAKE_INSTALL_PREFIX )
  SET( INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX} )
ELSE( DEFINED CMAKE_INSTALL_PREFIX )
  SET( INSTALL_PREFIX /usr )
ENDIF( DEFINED CMAKE_INSTALL_PREFIX )

# system configuration dir (etc)
IF( NOT DEFINED MANDIR )
  SET( MANDIR ${INSTALL_PREFIX}/share/man )
ENDIF( NOT DEFINED MANDIR )
MESSAGE( "** Manual files will be installed in ${MANDIR}" )

####################################################################
# CONFIGURATION                                                    #
####################################################################

IF( NOT DEFINED DOC_INSTALL_DIR )
  SET( DOC_INSTALL_DIR
     "${CMAKE_INSTALL_PREFIX}/share/doc/packages/${PACKAGE}"
     CACHE PATH "The install dir for documentation (default prefix/share/doc/packages/${PACKAGE})"
     FORCE
  )
ENDIF( NOT DEFINED DOC_INSTALL_DIR )

####################################################################
# INCLUDES                                                         #
####################################################################

#SET (CMAKE_INCLUDE_DIRECTORIES_BEFORE ON)
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} SYSTEM )


####################################################################
# RPM SPEC                                                         #
####################################################################

MACRO(SPECFILE)
  MESSAGE(STATUS "Writing spec file...")
  CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/${PACKAGE}.spec.cmake ${CMAKE_BINARY_DIR}/package/${PACKAGE}.spec @ONLY)
  MESSAGE(STATUS "I hate you rpm-lint...!!!")
  IF (EXISTS ${CMAKE_SOURCE_DIR}/package/${PACKAGE}-rpmlint.cmake)
    CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/package/${PACKAGE}-rpmlint.cmake ${CMAKE_BINARY_DIR}/package/${PACKAGE}-rpmlintrc @ONLY)
  ENDIF (EXISTS ${CMAKE_SOURCE_DIR}/package/${PACKAGE}-rpmlint.cmake)
ENDMACRO(SPECFILE)

MACRO(PKGCONFGFILE)
  MESSAGE(STATUS "Writing pkg-config file...")
  CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/libzypp.pc.cmake ${CMAKE_BINARY_DIR}/libzypp.pc @ONLY)
  INSTALL( FILES ${CMAKE_BINARY_DIR}/libzypp.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig )
ENDMACRO(PKGCONFGFILE)

####################################################################
# INSTALL                                                          #
####################################################################

MACRO(GENERATE_PACKAGING PACKAGE VERSION)

  # The following components are regex's to match anywhere (unless anchored)
  # in absolute path + filename to find files or directories to be excluded
  # from source tarball.
  SET (CPACK_SOURCE_IGNORE_FILES
  # hidden files
  "/\\\\..+$"
  # temporary files
  "\\\\.swp$"
  # backup files
  "~$"
  # others
  "\\\\.#"
  "/#"
  "/build/"
  "/_build/"
  # used before
  "/CVS/"
  "\\\\.o$"
  "\\\\.lo$"
  "\\\\.la$"
  "Makefile\\\\.in$"
  )

  #SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Novell's package management core engine.")
  SET(CPACK_PACKAGE_VENDOR "Novell Inc.")
  #SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/ReadMe.txt")
  #SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/Copyright.txt")
  #SET(CPACK_PACKAGE_VERSION_MAJOR ${version_major})
  #SET(CPACK_PACKAGE_VERSION_MINOR ${version_minor})
  #SET(CPACK_PACKAGE_VERSION_PATCH ${version_patch})
  SET( CPACK_GENERATOR "TBZ2")
  SET( CPACK_SOURCE_GENERATOR "TBZ2")
  SET( CPACK_SOURCE_PACKAGE_FILE_NAME "${PACKAGE}-${VERSION}" )
  INCLUDE(CPack)

  SPECFILE()

  ADD_CUSTOM_TARGET( svncheck
    COMMAND cd ${CMAKE_SOURCE_DIR} && LC_ALL=C git status | grep -q "nothing to commit .working directory clean."
  )

  SET( AUTOBUILD_COMMAND
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/package/*.tar.bz2
    COMMAND ${CMAKE_MAKE_PROGRAM} package_source
    COMMAND ${CMAKE_COMMAND} -E copy ${CPACK_SOURCE_PACKAGE_FILE_NAME}.tar.bz2 ${CMAKE_BINARY_DIR}/package
    COMMAND ${CMAKE_COMMAND} -E remove ${CPACK_SOURCE_PACKAGE_FILE_NAME}.tar.bz2
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/package/${PACKAGE}.changes" "${CMAKE_BINARY_DIR}/package/${PACKAGE}.changes"
  )

  ADD_CUSTOM_TARGET( srcpackage_local
    ${AUTOBUILD_COMMAND}
  )

  ADD_CUSTOM_TARGET( srcpackage
    COMMAND ${CMAKE_MAKE_PROGRAM} svncheck
    ${AUTOBUILD_COMMAND}
  )
ENDMACRO(GENERATE_PACKAGING)
