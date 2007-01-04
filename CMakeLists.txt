PROJECT(libzypp)

####################################################################
# CONFIGURATION                                                    #
####################################################################

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

INCLUDE(${CMAKE_SOURCE_DIR}/VERSION.cmake)
MATH ( EXPR LIBZYPP_CURRENT "${LIBZYPP_MAJOR} * 100 + ${LIBZYPP_MINOR}" )
MATH ( EXPR LIBZYPP_AGE "${LIBZYPP_MINOR} - ${LIBZYPP_COMPATMINOR}" )
SET ( LIBZYPP_VERSION_INFO "-version-info ${LIBZYPP_CURRENT}:${LIBZYPP_PATCH}:${LIBZYPP_AGE}" )

SET(VERSION "${LIBZYPP_MAJOR}.${LIBZYPP_MINOR}.${LIBZYPP_PATCH}")

ADD_DEFINITIONS( -DVERSION=\\\"${VERSION}\\\" )

FIND_PACKAGE(Boost REQUIRED)
FIND_PACKAGE(Gettext REQUIRED)
FIND_PACKAGE(Hal REQUIRED)
FIND_PACKAGE(Dbus REQUIRED)
FIND_PACKAGE(Glib REQUIRED)

IF (HAL_FOUND)
  MESSAGE( STATUS "Hal found" )
ELSE(HAL_FOUND)
  MESSAGE( ERROR "Hal not found" )
ENDIF(HAL_FOUND)

IF (GLIB_FOUND)
  MESSAGE( STATUS "glib found" )
ELSE(GLIB_FOUND)
  MESSAGE( ERROR "glib not found" )
ENDIF(GLIB_FOUND)

IF (DBUS_GLIB_FOUND)
  MESSAGE( STATUS "dbus-glib found" )
ELSE(DBUS_GLIB_FOUND)
  MESSAGE( ERROR "dbus-glib not found" )
ENDIF(DBUS_GLIB_FOUND)

MESSAGE(STATUS "soname: ${LIBZYPP_CURRENT}.${LIBZYPP_PATCH}.${LIBZYPP_AGE}")
MESSAGE(STATUS "version: ${VERSION}")
#yumschemadir=${zyppdir}/schema/yum
#xmlstoreschemadir=${zyppdir}/schema/xmlstore

####################################################################
# RPM SPEC                                                         #
####################################################################

MESSAGE(STATUS "Writing spec file...")
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/libzypp.spec.in ${CMAKE_BINARY_DIR}/package/libzypp.spec @ONLY)

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
#ADD_SUBDIRECTORY( examples ) 
#ADD_SUBDIRECTORY( tools ) 
#ADD_SUBDIRECTORY( testsuite ) 
#ADD_SUBDIRECTORY( po ) 
#ADD_SUBDIRECTORY( doc ) 

####################################################################
# INCLUDES                                                         #
####################################################################

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

####################################################################
# OTHERS                                                           #
####################################################################

#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )

####################################################################
# INSTALL                                                          #
####################################################################

#INSTALL_FILES(  FILES  libzypp.pc )
