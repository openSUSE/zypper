cmake_minimum_required(VERSION 3.1)
PROJECT(LIBZYPP)
SET( PACKAGE "libzypp" )
# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
SET( CMAKE_MODULE_PATH ${LIBZYPP_SOURCE_DIR}/cmake/modules )
INCLUDE(ZyppCommon)

OPTION (ENABLE_BUILD_DOCS "Build documentation by default?" OFF)
OPTION (ENABLE_BUILD_TRANS "Build translation files by default?" OFF)
OPTION (ENABLE_BUILD_TESTS "Build and run test suite by default?" OFF)
OPTION (ENABLE_ZSTD_COMPRESSION "Build with zstd compression support?" OFF)
OPTION (ENABLE_ZCHUNK_COMPRESSION "Build with zchunk compression support?" OFF)
# Helps with bug https://bugzilla.gnome.org/show_bug.cgi?id=784550 , Segfault during signal emission when slots are cleared
OPTION (ENABLE_SIGC_BLOCK_WORKAROUND "Enable a workaround for older sigcpp libraries?" OFF )
OPTION (DISABLE_MEDIABACKEND_TESTS "Disable Tests depending on Nginx and libfcgi?" OFF)

OPTION (DISABLE_LIBPROXY "Build without libproxy support even if package is installed?" OFF)
OPTION (DISABLE_AUTODOCS "Do not require doxygen being installed (required to build autodocs)?" OFF)
OPTION (EXPORT_NG_API "Export experimental libzypp API" OFF)
# This option will reroute all tool binaries to the libzypp build dir instead of taking those installed in the default directories.
OPTION (ENABLE_DEVEL_BUILD "Developer build, use zypp tools directly from build dir rather than the default locations" OFF)
OPTION (INSTALL_NG_BINARIES "Installs the NG binaries, disabled for now since we are not actively using them." OFF)
# Distros using just zypper may want to enable this as default earlier
OPTION (ENABLE_PREVIEW_SINGLE_RPMTRANS_AS_DEFAULT_FOR_ZYPPER "[preview] Force zypper into using a single rpm transaction to install" OFF)
#--------------------------------------------------------------------------------
SET (have_system x)

IF (DEBIAN)
  MESSAGE (STATUS "Building for Debian")
  SET (ENABLE_BUILD_DOCS ON)
  SET (ENABLE_BUILD_TRANS ON)
  SET (ENABLE_BUILD_TESTS ON)
  SET (have_system ${have_system}x)
ENDIF (DEBIAN)

IF ( ENABLE_DEVEL_BUILD )
        MESSAGE( WARNING "Zypp devel build enabled, do not do this in production" )
        SET( ZYPP_RPM_BINARY "${CMAKE_BINARY_DIR}/tools/zypp-rpm/zypp-rpm")
        SET( ZYPP_WORKER_PATH "${CMAKE_BINARY_DIR}/tools/workers" )
ELSE()
        SET( ZYPP_RPM_BINARY "${ZYPP_LIBEXEC_INSTALL_DIR}/zypp-rpm")
        SET( ZYPP_WORKER_PATH "${ZYPP_LIBEXEC_INSTALL_DIR}/workers" )
ENDIF( ENABLE_DEVEL_BUILD )

message ( "Using zypp-rpm from path: ${ZYPP_RPM_BINARY}" )
add_definitions( -DZYPP_RPM_BINARY="${ZYPP_RPM_BINARY}"  )
add_definitions( -DZYPP_WORKER_PATH="${ZYPP_WORKER_PATH}")

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

find_package(FindPkgConfig, QUIET)
INCLUDE( ${LIBZYPP_SOURCE_DIR}/VERSION.cmake )

MATH( EXPR LIBZYPP_NUMVERSION "${LIBZYPP_MAJOR} * 10000 + ${LIBZYPP_MINOR} * 100 + ${LIBZYPP_PATCH}" )
MATH( EXPR LIBZYPP_CURRENT "${LIBZYPP_MAJOR} * 100 + ${LIBZYPP_MINOR}" )
MATH( EXPR LIBZYPP_AGE     "${LIBZYPP_MINOR} - ${LIBZYPP_COMPATMINOR}" )
# Libtool wanted current:patch:age
# But cmake is not libtool, it wants the verbatim suffix to libzypp.so
MATH( EXPR LIBZYPP_SO_FIRST  "${LIBZYPP_CURRENT}-${LIBZYPP_AGE}" )
SET( VERSION "${LIBZYPP_MAJOR}.${LIBZYPP_MINOR}.${LIBZYPP_PATCH}" )
CONFIGURE_FILE ( zypp/APIConfig.h.in zypp/APIConfig.h )

SET (CMAKE_CXX_STANDARD ${ZYPPCOMMON_CXX_STANDARD})
SET (CMAKE_CXX_EXTENSIONS OFF)
include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
CHECK_C_COMPILER_FLAG("-Werror=format-security" CC_FORMAT_SECURITY)
CHECK_CXX_COMPILER_FLAG("-Werror=format-security" CXX_FORMAT_SECURITY)

SET( CMAKE_THREAD_PREFER_PTHREAD TRUE )
FIND_PACKAGE( Threads REQUIRED )
IF ( NOT CMAKE_USE_PTHREADS_INIT )
  MESSAGE( FATAL_ERROR "No pthreads found" )
ENDIF ( NOT CMAKE_USE_PTHREADS_INIT )

SET( CMAKE_C_FLAGS     "${CMAKE_C_FLAGS} -pthread -fno-strict-aliasing -fPIC -g -Wall -Wp,-D_GLIBCXX_ASSERTIONS" )
SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -fno-strict-aliasing -fPIC -g -Wall -Wp,-D_GLIBCXX_ASSERTIONS" )

SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility-inlines-hidden -Woverloaded-virtual -Wnon-virtual-dtor" )

set( CMAKE_C_FLAGS_RELEASE     "${CMAKE_C_FLAGS} -O3 -DZYPP_NDEBUG -DNDEBUG" )
set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3 -DZYPP_NDEBUG -DNDEBUG" )

IF(${CC_FORMAT_SECURITY})
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror=format-security")
ENDIF(${CC_FORMAT_SECURITY})

IF(${CXX_FORMAT_SECURITY})
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=format-security")
ENDIF(${CXX_FORMAT_SECURITY})

if ( NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  #cli args ignored by clang, it still prints a noisy warning though
  SET( CMAKE_C_FLAGS     "${CMAKE_C_FLAGS} -rdynamic -Wl,-as-needed" )
  SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic -Wl,-as-needed" )
endif()

SET( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pie" )

INCLUDE(CheckFunctionExists)
CHECK_FUNCTION_EXISTS(pipe2 PIPE2_FOUND)
IF(${PIPE2_FOUND})
  ADD_DEFINITIONS(-DHAVE_PIPE2)
ENDIF(${PIPE2_FOUND})

ADD_DEFINITIONS( -D_FILE_OFFSET_BITS=64 )
SET( LIBZYPP_VERSION_INFO "${LIBZYPP_SO_FIRST}.${LIBZYPP_AGE}.${LIBZYPP_PATCH}" )
SET( LIBZYPP_SOVERSION_INFO "${LIBZYPP_SO_FIRST}" )

GENERATE_PACKAGING(${PACKAGE} ${VERSION})

INCLUDE(CPack)

MACRO(ADD_TESTS)
  FOREACH( loop_var ${ARGV} )
    SET_SOURCE_FILES_PROPERTIES( ${loop_var}_test.cc COMPILE_FLAGS "-DBOOST_TEST_DYN_LINK -DBOOST_TEST_MAIN -DBOOST_AUTO_TEST_MAIN=\"\" " )
    ADD_EXECUTABLE( ${loop_var}_test ${loop_var}_test.cc )

    SET(TEST_REQ_LIBS zypp-allsym ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} )
    IF(NOT DISABLE_MEDIABACKEND_TESTS)
      LIST( APPEND TEST_REQ_LIBS zypp_test_utils)
    ENDIF()

    TARGET_LINK_LIBRARIES( ${loop_var}_test ${TEST_REQ_LIBS} )
    ADD_TEST( ${loop_var}_test ${CMAKE_CURRENT_BINARY_DIR}/${loop_var}_test --catch_system_errors=no)
  ENDFOREACH( loop_var )
ENDMACRO(ADD_TESTS)

# Macro to set the log group for a list of files
MACRO( SET_LOGGROUP _group _files  )
  SET_SOURCE_FILES_PROPERTIES( ${_files} COMPILE_FLAGS -DZYPP_BASE_LOGGER_LOGGROUP=\\"${_group}\\" )
  FOREACH (_currentFile ${ARGN})
#MESSAGE( STATUS "setting loggroup to \"${_group}\" for ${_currentFile}" )
    SET_SOURCE_FILES_PROPERTIES( ${_currentFile} COMPILE_FLAGS -DZYPP_BASE_LOGGER_LOGGROUP=\\"${_group}\\" )
  ENDFOREACH (_currentFile ${ARGN})
ENDMACRO( SET_LOGGROUP )

####################################################################

INCLUDE(CheckSymbolExists)
FIND_PACKAGE(Rpm REQUIRED)
IF ( NOT RPM_FOUND)
  MESSAGE( FATAL_ERROR " rpm-devel not found" )
ELSE ( NOT RPM_FOUND)
  INCLUDE_DIRECTORIES(${RPM_INCLUDE_DIR})
  # fix includes not relative to rpm
  INCLUDE_DIRECTORIES(${RPM_INCLUDE_DIR}/rpm)

  # rpm verify function and callback states were introduced in rpm-4.15
  if( RPM_LIB_VER VERSION_GREATER_EQUAL "4.15.0"  AND  RPM_LIB_VER VERSION_LESS "5.0.0")
    ADD_DEFINITIONS( -DHAVE_RPM_VERIFY_TRANSACTION_STEP )
  endif()

  if( RPM_LIB_VER VERSION_GREATER_EQUAL "5.0.0" )
        MESSAGE( STATUS "rpm found: enable rpm-4 compat interface." )
        ADD_DEFINITIONS(-D_RPM_5)
  endif ()

ENDIF( NOT RPM_FOUND)

FIND_PACKAGE(Boost REQUIRED COMPONENTS program_options unit_test_framework thread)
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

# CMake's own FindProtobuf is not compatible with protobuf >= 22,
# so force using protobuf's own CMake module.
FIND_PACKAGE(Protobuf CONFIG)
if ( NOT Protobuf_FOUND)
    # Protobuf's own CMake module not found, fall back to CMake's
    FIND_PACKAGE(Protobuf REQUIRED)
endif()
SET( Protobuf_IMPORT_DIRS ${CMAKE_CURRENT_SOURCE_DIR} )
if ( "${Protobuf_VERSION}" VERSION_LESS "3.8.0")
  message("Protobuf ${Protobuf_VERSION} has no ZeroCopyStreams, using internal version.")
  add_definitions( -DPROTOBUFLITE_HAS_NO_ZEROCOPYSTREAM )
endif()

FIND_PACKAGE(YAML-CPP REQUIRED)
IF ( NOT YAML-CPP_FOUND)
  MESSAGE( FATAL_ERROR " yaml-cpp not found" )
ELSE ( NOT YAML-CPP_FOUND)
  INCLUDE_DIRECTORIES(${YAML_CPP_INCLUDE_DIR})
  IF ( NOT YAML_CPP_LIBRARIES )
    # Must have been found by config insted of module, use target instead.
    SET( YAML_CPP_LIBRARIES "yaml-cpp" )
  ENDIF ( NOT YAML_CPP_LIBRARIES )
ENDIF( NOT YAML-CPP_FOUND)


#Allow to override the libsolv install location
IF( LIBSOLV_SRCDIR AND LIBSOLV_BUILDDIR )
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/libsolv
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/libsolv/include
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/libsolv/ext
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/libsolv/solvversion
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${LIBSOLV_SRCDIR}/src ${CMAKE_BINARY_DIR}/libsolv/include/solv
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${LIBSOLV_SRCDIR}/ext ${CMAKE_BINARY_DIR}/libsolv/ext/solv
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${LIBSOLV_BUILDDIR}/src ${CMAKE_BINARY_DIR}/libsolv/solvversion/solv
  )

  INCLUDE_DIRECTORIES( ${CMAKE_BINARY_DIR}/libsolv/include ${CMAKE_BINARY_DIR}/libsolv/ext ${CMAKE_BINARY_DIR}/libsolv/solvversion ${CMAKE_BINARY_DIR}/libsolv/solvversion/solv ${CMAKE_BINARY_DIR}/libsolv/ext/solv ${CMAKE_BINARY_DIR}/libsolv/include/solv )
  SET( LibSolv_LIBRARIES ${LIBSOLV_BUILDDIR}/ext/libsolvext.a ${LIBSOLV_BUILDDIR}/src/libsolv.a )

ELSE()
  FIND_PACKAGE(LibSolv REQUIRED ext)
  #SET(LibSolv_USE_STATIC_LIBS ON)
  IF ( NOT LibSolv_FOUND )
    MESSAGE( FATAL_ERROR " libsolv not found" )
  ELSE()
    INCLUDE_DIRECTORIES( ${LibSolv_INCLUDE_DIRS} )
    # static case: Assert libsolv.a ist the last one linked
    SET(LibSolv_LIBRARIES ${LibSolv_LIBRARIES} ${LIBSOLV_LIBRARY})
  ENDIF()
ENDIF( LIBSOLV_SRCDIR AND LIBSOLV_BUILDDIR )

FIND_PACKAGE(Gpgme REQUIRED)
IF ( NOT GPGME_PTHREAD_FOUND )
  MESSAGE( FATAL_ERROR " gpgme not found" )
ELSE()
  INCLUDE_DIRECTORIES( ${GPGME_INCLUDES} )
  LINK_DIRECTORIES(${GPGME_LIBRARY_DIR})
ENDIF()

FIND_PACKAGE(OpenSSL REQUIRED)

FIND_PACKAGE(Udev)
IF ( NOT UDEV_FOUND )
  MESSAGE(WARNING "No udev found. CD device detection will be poor")
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
    ADD_DEFINITIONS(-DWITH_LIBPROXY_SUPPORT)
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

# By now just libsolvs external references require us to link against it:
IF (ENABLE_ZSTD_COMPRESSION)
  MESSAGE("Building with zstd support enabled.")
  FIND_LIBRARY (ZSTD_LIBRARY NAMES zstd)
  FIND_PATH (ZSTD_INCLUDE_DIRS zstd.h)
  #INCLUDE_DIRECTORIES (${ZSTD_INCLUDE_DIRS})
ENDIF (ENABLE_ZSTD_COMPRESSION)

# https://bugzilla.gnome.org/show_bug.cgi?id=784550
IF (ENABLE_ZCHUNK_COMPRESSION)
  MESSAGE("Building with zchunk support enabled.")
  PKG_CHECK_MODULES (ZCHUNK zck REQUIRED)
  SET( CMAKE_C_FLAGS     "${CMAKE_C_FLAGS} ${ZCHUNK_CFLAGS}" )
  SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ZCHUNK_CFLAGS}" )
  ADD_DEFINITIONS (-DENABLE_ZCHUNK_COMPRESSION=1)
ENDIF(ENABLE_ZCHUNK_COMPRESSION)

IF(ENABLE_SIGC_BLOCK_WORKAROUND)
  message("Building with sigcpp block workaround")
  ADD_DEFINITIONS (-DLIBZYPP_USE_SIGC_BLOCK_WORKAROUND=1)
ENDIF(ENABLE_SIGC_BLOCK_WORKAROUND)

pkg_check_modules ( SIGCPP REQUIRED sigc++-2.0 )
INCLUDE_DIRECTORIES( ${SIGCPP_INCLUDE_DIRS} )

pkg_check_modules ( LIBGLIB REQUIRED glib-2.0 )
INCLUDE_DIRECTORIES( ${LIBGLIB_INCLUDE_DIRS} )

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

MESSAGE(STATUS "needreboot will be installed in ${SYSCONFDIR}/zypp/")
INSTALL( FILES ${LIBZYPP_SOURCE_DIR}/needreboot DESTINATION ${SYSCONFDIR}/zypp )

#install systemCheck
MESSAGE(STATUS "systemCheck will be installed in ${SYSCONFDIR}/zypp")
INSTALL( FILES ${LIBZYPP_SOURCE_DIR}/systemCheck DESTINATION ${SYSCONFDIR}/zypp )

# logrotate config file
INSTALL( FILES ${LIBZYPP_SOURCE_DIR}/zypp-history.lr DESTINATION ${SYSCONFDIR}/logrotate.d )

IF ( ENABLE_PREVIEW_SINGLE_RPMTRANS_AS_DEFAULT_FOR_ZYPPER )
  MESSAGE( STATUS "[preview] Force zypper into using a single rpm transaction to install" )
  ADD_DEFINITIONS( -DSINGLE_RPMTRANS_AS_DEFAULT_FOR_ZYPPER )
ENDIF ( ENABLE_PREVIEW_SINGLE_RPMTRANS_AS_DEFAULT_FOR_ZYPPER )


####################################################################
# SUBDIRECTORIES                                                   #
####################################################################
ADD_SUBDIRECTORY( zypp-proto )
ADD_SUBDIRECTORY( zypp-core )
ADD_SUBDIRECTORY( zypp-media )
ADD_SUBDIRECTORY( zypp-curl )
ADD_SUBDIRECTORY( zypp-tui )
ADD_SUBDIRECTORY( zypp )
ADD_SUBDIRECTORY( tools )
ADD_SUBDIRECTORY( doc )

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
