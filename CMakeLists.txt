PROJECT(libzypp)

####################################################################
# CONFIGURATION                                                    #
####################################################################

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
SET( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

INCLUDE( ${CMAKE_SOURCE_DIR}/VERSION.cmake )
MATH( EXPR LIBZYPP_CURRENT "${LIBZYPP_MAJOR} * 100 + ${LIBZYPP_MINOR}" )
MATH( EXPR LIBZYPP_AGE     "${LIBZYPP_MINOR} - ${LIBZYPP_COMPATMINOR}" )

SET( PACKAGE "libzypp" )
SET( VERSION "${LIBZYPP_MAJOR}.${LIBZYPP_MINOR}.${LIBZYPP_PATCH}" )

ADD_DEFINITIONS( -DVERSION=\\\"${VERSION}\\\" )
SET( LIBZYPP_VERSION_INFO   "${LIBZYPP_CURRENT}.${LIBZYPP_PATCH}.${LIBZYPP_AGE}" )


SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Novell's package management core engine.")
SET(CPACK_PACKAGE_VENDOR "Novell Inc.")
#SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/ReadMe.txt")
#SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/Copyright.txt")
SET(CPACK_PACKAGE_VERSION_MAJOR ${LIBZYPP_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${LIBZYPP_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${LIBZYPP_PATCH})
SET( CPACK_GENERATOR "TBZ2")
SET( CPACK_SOURCE_GENERATOR "TBZ2")
SET( CPACK_SOURCE_PACKAGE_FILE_NAME "${PACKAGE}-${VERSION}" )

# The following components are regex's to match anywhere (unless anchored)
# in absolute path + filename to find files or directories to be excluded
# from source tarball.
SET (CPACK_SOURCE_IGNORE_FILES
#/CVS/;/\\.svn/;/\\.libs/;/\\.deps/;\\.swp$;/build/;\\.o$;\\.lo$;\\.la$;~$;\\\\.cvsignore$;Makefile\\\\.in$;
/CVS/;/\\\\.svn/;/\\\\.libs/;/\\\\.deps/;\\\\.swp$;/build/;\\\\.o$;\\\\.lo$;\\\\.la$;~$
)

INCLUDE(CPack)

####################################################################

INCLUDE(DefineInstallationPaths)

SET( DOC_INSTALL_DIR
   "${SHARE_INSTALL_PREFIX}/doc/packages/${PACKAGE}"
   CACHE PATH "The install dir for documentation (default prefix/share/doc/packages/${PACKAGE})"
   FORCE
)

####################################################################

FIND_PACKAGE(Gettext REQUIRED)

FIND_PACKAGE(Rpm REQUIRED)
IF ( NOT RPM_FOUND)
  MESSAGE( FATAL_ERROR " rpm not found" )
ELSE ( NOT RPM_FOUND)
  INCLUDE_DIRECTORIES(${RPM_INCLUDE_DIR})
ENDIF( NOT RPM_FOUND)

FIND_PACKAGE(Boost REQUIRED)
IF (Boost_FOUND)
  MESSAGE( STATUS "boost found: includes in ${Boost_INCLUDE_DIRS}, library in ${Boost_LIBRARY_DIRS}")
  INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
  LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
ENDIF(Boost_FOUND)

FIND_PACKAGE(Gettext REQUIRED)
IF (GETTEXT_FOUND)
  MESSAGE(STATUS "Found Gettext: ${GETTEXT_SOURCE}")
  INCLUDE_DIRECTORIES(${GETTEXT_INCLUDE_DIR})
ELSE (GETTEXT_FOUND)
  MESSAGE( FATAL_ERROR "Gettext not found" )
ENDIF (GETTEXT_FOUND)

FIND_PACKAGE(Hal REQUIRED)
IF ( NOT HAL_FOUND)
  MESSAGE( FATAL_ERROR " Hal not found" )
ELSE ( NOT HAL_FOUND)
  INCLUDE_DIRECTORIES(${HAL_INCLUDE_DIR})
# hal/libhal-storage.h is broken grrr
  INCLUDE_DIRECTORIES(${HAL_INCLUDE_DIR}/hal)
ENDIF( NOT HAL_FOUND)

FIND_PACKAGE(Dbus REQUIRED)
IF ( NOT DBUS_FOUND)
  MESSAGE( FATAL_ERROR " dbus not found" )
ELSE ( NOT DBUS_FOUND)
  INCLUDE_DIRECTORIES(${DBUS_INCLUDE_DIR})
  INCLUDE_DIRECTORIES(${DBUS_ARCH_INCLUDE_DIR})
ENDIF( NOT DBUS_FOUND)

FIND_PACKAGE(Glib REQUIRED)
IF (GLIB_FOUND)
  MESSAGE( STATUS "glib found: includes in ${GLIB_INCLUDE_DIR}, library in ${GLIB_LIBRARY}")
  INCLUDE_DIRECTORIES(${GLIB_INCLUDE_DIR})
  INCLUDE_DIRECTORIES(${GLIB_CONFIG_INCLUDE_DIR})
ELSE(GLIB_FOUND)
  MESSAGE( FATAL_ERROR "glib not found" )
ENDIF(GLIB_FOUND)

#IF (DBUS_GLIB_FOUND)
#  MESSAGE( STATUS "dbus-glib found" )
#  INCLUDE_DIRECTORIES(${DBUS_GLIB_INCLUDE_DIR})
#  LINK_DIRECTORIES(${DBUS_GLIB_LINK_DIR})
#ELSE(DBUS_GLIB_FOUND)
#  MESSAGE( ERROR "dbus-glib not found" )
#ENDIF(DBUS_GLIB_FOUND)

FIND_PACKAGE(Curl REQUIRED)
IF ( NOT CURL_FOUND)
  MESSAGE( FATAL_ERROR " curl not found" )
ELSE ( NOT CURL_FOUND)
  INCLUDE_DIRECTORIES(${CURL_INCLUDE_DIR})
ENDIF( NOT CURL_FOUND)

FIND_PACKAGE(Libxml REQUIRED)
IF ( NOT LIBXML_FOUND)
  MESSAGE( FATAL_ERROR " libxml not found" )
ELSE ( NOT LIBXML_FOUND)
  INCLUDE_DIRECTORIES(${LIBXML_INCLUDE_DIR})
ENDIF( NOT LIBXML_FOUND)

FIND_PACKAGE(Sqlite REQUIRED)

FIND_PACKAGE(ZLIB REQUIRED)
IF ( NOT ZLIB_FOUND)
  MESSAGE( FATAL_ERROR " zlib not found" )
ELSE ( NOT ZLIB_FOUND)
  INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
ENDIF( NOT ZLIB_FOUND)

FIND_PROGRAM( DOXYGEN doxygen )
IF ( NOT DOXYGEN )
  MESSAGE( FATAL_ERROR "doxygen not found: install doxygen to build the documentation." )
ELSE ( NOT DOXYGEN )
  MESSAGE( STATUS "doxygen found: ${DOXYGEN}" )
ENDIF ( NOT DOXYGEN )

MESSAGE(STATUS "soname: ${LIBZYPP_CURRENT}.${LIBZYPP_PATCH}.${LIBZYPP_AGE}")
MESSAGE(STATUS "version: ${VERSION}")

####################################################################
# RPM SPEC                                                         #
####################################################################

MESSAGE(STATUS "Writing spec file...")
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/libzypp.spec.cmake ${CMAKE_BINARY_DIR}/package/libzypp.spec @ONLY)

MESSAGE(STATUS "Writing pkg-config file...")
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/libzypp.pc.cmake ${CMAKE_BINARY_DIR}/libzypp.pc @ONLY)

INSTALL( FILES ${CMAKE_BINARY_DIR}/libzypp.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig )

MACRO(add_subdir_sources subdir real_sources)
  MESSAGE( STATUS "Adding ${subdir} to sources..." )
  SET(sources)
  INCLUDE("${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/CMakeLists.txt")
  FOREACH(src ${sources})
    MESSAGE( STATUS "  - ${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/${src}" )
    SET(real_sources ${real_sources} "${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/${src}")
  ENDFOREACH(src)
ENDMACRO(add_subdir_sources)

####################################################################
# SUBDIRECTORIES                                                   #
####################################################################

ADD_SUBDIRECTORY( zypp )
ADD_SUBDIRECTORY( zypp2 )
# do not build devel by default
ADD_SUBDIRECTORY( devel EXCLUDE_FROM_ALL )
ADD_SUBDIRECTORY( tools )
#ADD_SUBDIRECTORY( examples )
ADD_SUBDIRECTORY( po EXCLUDE_FROM_ALL )
ADD_SUBDIRECTORY( doc EXCLUDE_FROM_ALL )

####################################################################
# INCLUDES                                                         #
####################################################################

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

####################################################################
# OTHERS                                                           #
####################################################################

#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )
#yumschemadir=${zyppdir}/schema/yum
#xmlstoreschemadir=${zyppdir}/schema/xmlstore

####################################################################
# INSTALL                                                          #
####################################################################

#INSTALL_FILES(  FILES  libzypp.pc )

ADD_CUSTOM_TARGET( svncheck
  COMMAND cd $(CMAKE_SOURCE_DIR) && ! LC_ALL=C svn status --show-updates --quiet | grep -v '^Status against revision'
)

SET( AUTOBUILD_COMMAND
  COMMAND ${CMAKE_MAKE_PROGRAM} package_source
  COMMAND ${CMAKE_COMMAND} -E copy ${CPACK_SOURCE_PACKAGE_FILE_NAME}.tar.bz2 ${CMAKE_BINARY_DIR}/package
  COMMAND ${CMAKE_COMMAND} -E remove ${CPACK_SOURCE_PACKAGE_FILE_NAME}.tar.bz2
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/package/${PACKAGE}.changes" "${CMAKE_BINARY_DIR}/package/${PACKAGE}.changes"
)

ADD_CUSTOM_TARGET( autobuild_local
  ${AUTOBUILD_COMMAND}
)

ADD_CUSTOM_TARGET( autobuild
  COMMAND ${CMAKE_MAKE_PROGRAM} svncheck
  ${AUTOBUILD_COMMAND}
)
