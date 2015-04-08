PROJECT(LIBZYPP)
SET( PACKAGE "libzypp" )
# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
SET( CMAKE_MODULE_PATH ${LIBZYPP_SOURCE_DIR}/cmake/modules )
cmake_minimum_required(VERSION 2.6)

OPTION (ENABLE_BUILD_DOCS "Build documentation by default?" OFF)
OPTION (ENABLE_BUILD_TRANS "Build translation files by default?" OFF)
OPTION (ENABLE_BUILD_TESTS "Build and run test suite by default?" OFF)

OPTION (DISABLE_LIBPROXY "Build without libproxy support even if package is installed?" OFF)
OPTION (DISABLE_AUTODOCS "Do not require doxygen being installed (required to build autodocs)?" OFF)

#--------------------------------------------------------------------------------
SET (have_system x)

IF (DEBIAN)
  MESSAGE (STATUS "Building for Debian")
  SET (ENABLE_BUILD_DOCS ON)
  SET (ENABLE_BUILD_TRANS ON)
  SET (ENABLE_BUILD_TESTS ON)
  SET (have_system ${have_system}x)
ENDIF (DEBIAN)

IF (${have_system} STREQUAL x)
  MESSAGE (STATUS "Building for SUSE")
ENDIF (${have_system} STREQUAL x)

IF (${have_system} STRGREATER xx)
  MESSAGE (FATAL_ERROR "Can only build for one system type.")
ENDIF (${have_system} STRGREATER xx)
#--------------------------------------------------------------------------------

# allow name libraries by name mixed with full
# paths
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

INCLUDE(ZyppCommon)
INCLUDE( ${LIBZYPP_SOURCE_DIR}/VERSION.cmake )

MATH( EXPR LIBZYPP_CURRENT "${LIBZYPP_MAJOR} * 100 + ${LIBZYPP_MINOR}" )
MATH( EXPR LIBZYPP_AGE     "${LIBZYPP_MINOR} - ${LIBZYPP_COMPATMINOR}" )
# Libtool wanted current:patch:age
# But cmake is not libtool, it wants the verbatim suffix to libzypp.so
MATH( EXPR LIBZYPP_SO_FIRST  "${LIBZYPP_CURRENT}-${LIBZYPP_AGE}" )
SET( VERSION "${LIBZYPP_MAJOR}.${LIBZYPP_MINOR}.${LIBZYPP_PATCH}" )

include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
CHECK_C_COMPILER_FLAG("-Werror=format-security" CC_FORMAT_SECURITY)
CHECK_CXX_COMPILER_FLAG("-Werror=format-security" CXX_FORMAT_SECURITY)

SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility-inlines-hidden -fno-strict-aliasing -fPIC -g -rdynamic -Wall -Woverloaded-virtual -Wnon-virtual-dtor -Wl,-as-needed -std=c++11" )
SET( CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -fno-strict-aliasing -fPIC -g -rdynamic -Wall -Wl,-as-needed" )

set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -O3 -DZYPP_NDEBUG" )
set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3 -DZYPP_NDEBUG" )

IF(${CC_FORMAT_SECURITY})
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror=format-security")
ENDIF(${CC_FORMAT_SECURITY})

IF(${CXX_FORMAT_SECURITY})
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=format-security")
ENDIF(${CXX_FORMAT_SECURITY})

INCLUDE(CheckFunctionExists)
CHECK_FUNCTION_EXISTS(pipe2 PIPE2_FOUND)
IF(${PIPE2_FOUND})
  ADD_DEFINITIONS(-DHAVE_PIPE2)
ENDIF(${PIPE2_FOUND})

ADD_DEFINITIONS( -D_FILE_OFFSET_BITS=64 )
ADD_DEFINITIONS( -DVERSION="${VERSION}" )
SET( LIBZYPP_VERSION_INFO "${LIBZYPP_SO_FIRST}.${LIBZYPP_AGE}.${LIBZYPP_PATCH}" )
SET( LIBZYPP_SOVERSION_INFO "${LIBZYPP_SO_FIRST}" )

GENERATE_PACKAGING(${PACKAGE} ${VERSION})

INCLUDE(CPack)

MACRO(ADD_TESTS)
  FOREACH( loop_var ${ARGV} )
    SET_SOURCE_FILES_PROPERTIES( ${loop_var}_test.cc COMPILE_FLAGS "-DBOOST_TEST_DYN_LINK -DBOOST_TEST_MAIN -DBOOST_AUTO_TEST_MAIN=\"\" " )
    ADD_EXECUTABLE( ${loop_var}_test ${loop_var}_test.cc )
    TARGET_LINK_LIBRARIES( ${loop_var}_test  zypp ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} zypp_test_utils)
    ADD_TEST( ${loop_var}_test ${CMAKE_CURRENT_BINARY_DIR}/${loop_var}_test --catch_system_errors=no)
  ENDFOREACH( loop_var )
ENDMACRO(ADD_TESTS)

####################################################################
# prefer packages using the same install prefix as we do
SET(CMAKE_PREFIX_PATH ${CMAKE_INSTALL_PREFIX} usr/localX /usr/local /usr)

FIND_PACKAGE(Rpm REQUIRED)
IF ( NOT RPM_FOUND)
  MESSAGE( FATAL_ERROR " rpm-devel not found" )
ELSE ( NOT RPM_FOUND)
  INCLUDE_DIRECTORIES(${RPM_INCLUDE_DIR})
  # fix includes not relative to rpm
  INCLUDE_DIRECTORIES(${RPM_INCLUDE_DIR}/rpm)
  if ( RPM_SUSPECT_VERSION STREQUAL "5.x" )
  	MESSAGE( STATUS "rpm found: enable rpm-4 compat interface." )
  	ADD_DEFINITIONS(-D_RPM_5)
  elseif ( RPM_SUSPECT_VERSION STREQUAL "4.x" )
	MESSAGE( STATUS "rpm found: use rpm-4.x interface." )
	ADD_DEFINITIONS(-D_RPM_4_X)
  elseif ( RPM_SUSPECT_VERSION STREQUAL "4.4" )
  	MESSAGE( STATUS "rpm found: enable rpm-4.4 legacy interface." )
	ADD_DEFINITIONS(-D_RPM_4_4)
  endif ( RPM_SUSPECT_VERSION STREQUAL "5.x" )
ENDIF( NOT RPM_FOUND)

FIND_PACKAGE(Boost REQUIRED COMPONENTS program_options thread unit_test_framework)
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

FIND_PACKAGE(CURL REQUIRED)
IF ( NOT CURL_FOUND)
  MESSAGE( FATAL_ERROR " curl not found" )
ELSE ( NOT CURL_FOUND)
  INCLUDE_DIRECTORIES(${CURL_INCLUDE_DIRS})
ENDIF( NOT CURL_FOUND)

FIND_PACKAGE(LibXml2 REQUIRED)
IF ( NOT LIBXML2_FOUND)
  MESSAGE( FATAL_ERROR " libxml not found" )
ELSE ( NOT LIBXML2_FOUND)
  INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})
ENDIF( NOT LIBXML2_FOUND)

FIND_PACKAGE(ZLIB REQUIRED)
IF ( NOT ZLIB_FOUND)
  MESSAGE( FATAL_ERROR " zlib not found" )
ELSE ( NOT ZLIB_FOUND)
  INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
ENDIF( NOT ZLIB_FOUND)

#SET(LibSolv_USE_STATIC_LIBS ON)
FIND_PACKAGE(LibSolv REQUIRED ext)
IF ( NOT LibSolv_FOUND )
  MESSAGE( FATAL_ERROR " libsolv not found" )
ELSE()
  INCLUDE_DIRECTORIES( ${LibSolv_INCLUDE_DIRS} )
ENDIF()

# libsolv uses expat and has open references to it:
FIND_PACKAGE(EXPAT REQUIRED)

FIND_PACKAGE(OpenSSL REQUIRED)

FIND_PACKAGE(Udev)
IF ( NOT UDEV_FOUND )
  FIND_PACKAGE(Hal)
  IF ( NOT HAL_FOUND )
    MESSAGE(WARNING "No udev or HAL. CD device detection will be poor")
  ELSE ( NOT HAL_FOUND )
      ADD_DEFINITIONS(-DHAVE_HAL)
      INCLUDE_DIRECTORIES(${HAL_INCLUDE_DIR})
      INCLUDE_DIRECTORIES(${HAL_INCLUDE_DIR}/hal)
      # HAL requires working dbus
      FIND_PACKAGE(Dbus REQUIRED)
      IF(DBUS_FOUND)
        INCLUDE_DIRECTORIES(${DBUS_INCLUDE_DIR})
        INCLUDE_DIRECTORIES(${DBUS_ARCH_INCLUDE_DIR})
      ENDIF(DBUS_FOUND)
  ENDIF ( NOT HAL_FOUND )
ELSE ( NOT UDEV_FOUND )
  ADD_DEFINITIONS(-DHAVE_UDEV)
ENDIF ( NOT UDEV_FOUND )

IF( DISABLE_LIBPROXY )
  MESSAGE( STATUS "libproxy support disabled" )
ELSE( DISABLE_LIBPROXY )
  FIND_PACKAGE(libproxy)
  IF ( NOT LIBPROXY_FOUND )
    MESSAGE( STATUS "libproxy not found" )
  ELSE ( NOT LIBPROXY_FOUND )
    INCLUDE_DIRECTORIES( ${LIBPROXY_INCLUDE_DIR} )
    ADD_DEFINITIONS(-D_WITH_LIBPROXY_SUPPORT_)
  ENDIF( NOT LIBPROXY_FOUND )
ENDIF( DISABLE_LIBPROXY )

FIND_PROGRAM( DOXYGEN doxygen )
IF ( NOT DOXYGEN )
  IF ( DISABLE_AUTODOCS )
    MESSAGE( STATUS "doxygen is not available. Can't build the documentation." )
  ELSE ( DISABLE_AUTODOCS )
    MESSAGE( FATAL_ERROR "doxygen not found: install doxygen to build the documentation!" )
  ENDIF ( DISABLE_AUTODOCS )
ELSE ( NOT DOXYGEN )
  MESSAGE( STATUS "doxygen found: ${DOXYGEN}" )
ENDIF ( NOT DOXYGEN )

MESSAGE(STATUS "soname: ${LIBZYPP_VERSION_INFO}")
MESSAGE(STATUS "version: ${VERSION}")

MESSAGE(STATUS "Writing pkg-config file...")
CONFIGURE_FILE(${LIBZYPP_SOURCE_DIR}/libzypp.pc.cmake ${LIBZYPP_BINARY_DIR}/libzypp.pc @ONLY)
INSTALL( FILES ${LIBZYPP_BINARY_DIR}/libzypp.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig )

MESSAGE(STATUS "FindZypp.cmake will be installed in ${CMAKE_INSTALL_PREFIX}/share/cmake/Modules")
INSTALL( FILES ${LIBZYPP_SOURCE_DIR}/cmake/modules/FindZypp.cmake DESTINATION ${CMAKE_INSTALL_PREFIX}/share/cmake/Modules )
INSTALL( FILES ${LIBZYPP_SOURCE_DIR}/cmake/modules/ZyppCommon.cmake DESTINATION ${CMAKE_INSTALL_PREFIX}/share/cmake/Modules )

####################################################################
# config templates
# (don't forget to mention them in the .spec file)
####################################################################

MESSAGE(STATUS "zypp.conf will be installed in ${SYSCONFDIR}/zypp")
INSTALL( FILES ${LIBZYPP_SOURCE_DIR}/zypp.conf DESTINATION ${SYSCONFDIR}/zypp )

#install systemCheck
MESSAGE(STATUS "systemCheck will be installed in ${SYSCONFDIR}/zypp")
INSTALL( FILES ${LIBZYPP_SOURCE_DIR}/systemCheck DESTINATION ${SYSCONFDIR}/zypp )

# logrotate config file
INSTALL( FILES ${LIBZYPP_SOURCE_DIR}/zypp-history.lr DESTINATION ${SYSCONFDIR}/logrotate.d )

####################################################################
# SUBDIRECTORIES                                                   #
####################################################################

ADD_SUBDIRECTORY( zypp )
# do not build devel by default
ADD_SUBDIRECTORY( devel EXCLUDE_FROM_ALL )
ADD_SUBDIRECTORY( tools )
ADD_SUBDIRECTORY( examples )
ADD_SUBDIRECTORY( doc )
ADD_SUBDIRECTORY( vendor )

IF ( ENABLE_BUILD_TRANS )
  ADD_SUBDIRECTORY( po )
ELSE ( ENABLE_BUILD_TRANS )
  ADD_SUBDIRECTORY( po EXCLUDE_FROM_ALL )
ENDIF ( ENABLE_BUILD_TRANS )

IF ( ENABLE_BUILD_TESTS )
  ADD_SUBDIRECTORY( tests )
ELSE ( ENABLE_BUILD_TESTS )
  ADD_SUBDIRECTORY( tests EXCLUDE_FROM_ALL )
ENDIF ( ENABLE_BUILD_TESTS )
INCLUDE(CTest)
ENABLE_TESTING()
