PROJECT( zypper C CXX )

# VERSION

INCLUDE( ${CMAKE_SOURCE_DIR}/VERSION.cmake )
SET( VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )
ADD_DEFINITIONS( -DVERSION=\\\"${VERSION}\\\" )

# usr INSTALL_PREFIX

IF( DEFINED CMAKE_INSTALL_PREFIX )
  SET( INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX} )
ELSE( DEFINED CMAKE_INSTALL_PREFIX )
  SET( INSTALL_PREFIX /usr )
ENDIF( DEFINED CMAKE_INSTALL_PREFIX )

# system configuration dir (etc)

IF( NOT DEFINED SYSCONFDIR )
  SET( SYSCONFDIR /etc )
ENDIF( NOT DEFINED SYSCONFDIR )

# system configuration dir (etc)

IF( NOT DEFINED MANDIR )
  SET( MANDIR ${CMAKE_INSTALL_PREFIX}/share/man )
ENDIF( NOT DEFINED MANDIR )


# PACKAGE

SET( PACKAGE "zypper" )

# CPack

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Command line package management tool.")
SET(CPACK_PACKAGE_VENDOR "Novell Inc.")
SET(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
SET(CPACK_GENERATOR "TBZ2")
SET(CPACK_SOURCE_GENERATOR "TBZ2")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${PACKAGE}-${VERSION}")

# The following components are regex's to match anywhere (unless anchored)
# in absolute path + filename to find files or directories to be excluded
# from source tarball.
SET (CPACK_SOURCE_IGNORE_FILES
# svn files
"/\\\\.svn/"
# temporary files
"\\\\.swp$"
# backup files
"~$"
"\\\\.bak$"
# eclipse files
"\\\\.settings/"
"\\\\.cdtproject"
"\\\\.project"
"\\\\.cproject"
# other
"/_build/"
)

INCLUDE(CPack)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
SET( CMAKE_MODULE_PATH
  # locally installed (development) version of libzypp
  ${ZYPP_PREFIX}/share/cmake/Modules
  # cmake modules not found in upstream
  ${CMAKE_SOURCE_DIR}/cmake/modules
)


FIND_PACKAGE( Zypp REQUIRED )
IF( ZYPP_FOUND )
  INCLUDE_DIRECTORIES(${ZYPP_INCLUDE_DIR})
ELSE( ZYPP_FOUND )
  MESSAGE( FATAL_ERROR "libzypp not found" )
ENDIF( ZYPP_FOUND )

FIND_PACKAGE( Gettext REQUIRED )
IF( GETTEXT_FOUND )
  INCLUDE_DIRECTORIES(${GETTEXT_INCLUDE_DIR})
ELSE( GETTEXT_FOUND )
  MESSAGE( FATAL_ERROR "gettext not found" )
ENDIF( GETTEXT_FOUND )

FIND_PACKAGE( Readline REQUIRED )
IF( READLINE_FOUND )
  INCLUDE_DIRECTORIES(${READLINE_INCLUDE_DIR})
ELSE( READLINE_FOUND )
  MESSAGE( FATAL_ERROR "readline not found" )
ENDIF( READLINE_FOUND )


# RPM spec file

MESSAGE(STATUS "Writing RPM spec file...")
CONFIGURE_FILE(
  ${CMAKE_SOURCE_DIR}/zypper.spec.cmake
  ${CMAKE_BINARY_DIR}/package/zypper.spec
  @ONLY)

MESSAGE(STATUS "version: ${VERSION}")

ADD_SUBDIRECTORY( src )
ADD_SUBDIRECTORY( po )
ADD_SUBDIRECTORY( doc )


# svn check

ADD_CUSTOM_TARGET( svncheck
  COMMAND cd $(CMAKE_SOURCE_DIR) && ! LC_ALL=C svn status --show-updates --quiet | grep -v '^Status against revision'
)

# generate source tarball, rpm spec file and copy them together with .changes
# file over to ${CMAKE_BINARY_DIR}/package dir

SET( PACKAGE_COMMAND
  COMMAND ${CMAKE_MAKE_PROGRAM} package_source
  COMMAND ${CMAKE_COMMAND} -E copy ${CPACK_SOURCE_PACKAGE_FILE_NAME}.tar.bz2 ${CMAKE_BINARY_DIR}/package
  COMMAND ${CMAKE_COMMAND} -E remove ${CPACK_SOURCE_PACKAGE_FILE_NAME}.tar.bz2
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/package/${PACKAGE}.changes" "${CMAKE_BINARY_DIR}/package/${PACKAGE}.changes"
)

# make source tarball without svn check

ADD_CUSTOM_TARGET( srcpackage_local
  ${PACKAGE_COMMAND}
)

# make source tarball with svn check

ADD_CUSTOM_TARGET( srcpackage
  COMMAND ${CMAKE_MAKE_PROGRAM} svncheck
  ${PACKAGE_COMMAND}
)

# documentation too important to be hidden in doc:
INSTALL(
  FILES COPYING HACKING
  DESTINATION ${INSTALL_PREFIX}/share/doc/packages/${PACKAGE}
)

