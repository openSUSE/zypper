CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
PROJECT( ZYPPER C CXX )
SET( PACKAGE "zypper" )

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
if ( ZYPP_STACK_BUILD )
  SET( ZYPPER_TARGET_PREFIX "zypper_")
  SET( CMAKE_MODULE_PATH
      ${zypp-libs_SOURCE_DIR}/cmake/modules/
      ${CMAKE_MODULE_PATH}
  )
  include_directories(${ZYPP_STACK_INCLUDE_DIRS})
else()
  if (DEFINED ZYPP_PREFIX)
    SET( CMAKE_MODULE_PATH
      ${ZYPP_PREFIX}/${LIB}/cmake/Zypp/
      ${ZYPP_PREFIX}/lib64/cmake/Zypp/
      ${ZYPP_PREFIX}/lib/cmake/Zypp/
      ${CMAKE_MODULE_PATH}
    )
  else()
    SET( CMAKE_MODULE_PATH
      ${CMAKE_INSTALL_PREFIX}/${LIB}/cmake/Zypp/
      ${CMAKE_INSTALL_PREFIX}/lib64/cmake/Zypp/
      ${CMAKE_INSTALL_PREFIX}/lib/cmake/Zypp/
      ${CMAKE_MODULE_PATH}
    )
  endif()
endif()

SET( CMAKE_MODULE_PATH
  ${ZYPPER_SOURCE_DIR}/cmake/modules
  ${CMAKE_MODULE_PATH}
)
INCLUDE(ZyppCommon)

OPTION (ENABLE_BUILD_TRANS "Build translation files by default?" ON)
OPTION (ENABLE_BUILD_TESTS "Build and run test suite by default?" OFF)

# VERSION
INCLUDE( ${ZYPPER_SOURCE_DIR}/VERSION.cmake )
SET( VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )
ADD_DEFINITIONS( -D_FILE_OFFSET_BITS=64 )
ADD_DEFINITIONS( -DVERSION="${VERSION}" )

SET( CMAKE_CXX_STANDARD ${ZYPPCOMMON_CXX_STANDARD} )
SET( CMAKE_CXX_EXTENSIONS OFF )
SET( CMAKE_CXX_FLAGS_RELEASE "-O3" )
SET( CMAKE_CXX_FLAGS_RELWITHDEBUGINFO "-g -O2" )
SET( CMAKE_C_FLAGS_RELEASE   "-O3" )
SET( CMAKE_C_FLAGS_RELWITHDEBUGINFO "-g -O2" )

SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpie -Wall -Woverloaded-virtual -Wnon-virtual-dtor -fvisibility=hidden" )
SET( CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -fpie -Wall" )

SET( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fpie -fvisibility=hidden" )

if( ENABLE_UBSAN_CHECKS )
  SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined -fsanitize=address" )
  SET( CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -fsanitize=undefined -fsanitize=address" )
  SET( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lubsan")
  SET( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -lubsan ")
endif()


IF ( LEGACY_ENABLE_LONGOPT_ABBREV )
  # Fixed in 1.14.34: Do not allow the abbreviation of cli arguments (bsc#1164543)
  # On SLE15/Leap 15.0 and 15.1 we will stay bug-compatible and accept the
  # abbreviations in order not to break tools. In 15.2 they must be fixed.
  MESSAGE( STATUS "LEGACY_ENABLE_LONGOPT_ABBREV" )
  ADD_DEFINITIONS( -DLEGACY_ENABLE_LONGOPT_ABBREV=1 )
endif()

GENERATE_PACKAGING(${PACKAGE} ${VERSION})

if ( ZYPP_STACK_BUILD )
  set( ZYPP_FOUND 1 )
  #set( ZYPP_INCLUDE_DIR ${LIBZYPP_SOURCE_DIR} ${zypp-libs_SOURCE_DIR}/zypp-logic  )
  set( ZYPP_LIBRARY zypp )
  set( ZYPP_TUI_LIBRARY zypp-tui )
else()
  FIND_PACKAGE( Zypp REQUIRED )
endif()

IF( ZYPP_FOUND )
  INCLUDE_DIRECTORIES(${ZYPP_INCLUDE_DIR})
ELSE( ZYPP_FOUND )
  MESSAGE( FATAL_ERROR "libzypp not found" )
ENDIF( ZYPP_FOUND )

FIND_PACKAGE( Gettext REQUIRED )
INCLUDE_DIRECTORIES(${GETTEXT_INCLUDE_DIR})

FIND_PACKAGE( Readline REQUIRED )
INCLUDE_DIRECTORIES(${READLINE_INCLUDE_DIR})

FIND_PACKAGE( Augeas REQUIRED )
INCLUDE_DIRECTORIES(${AUGEAS_INCLUDE_DIR})
FIND_PACKAGE(LibXml2)
IF (LIBXML2_FOUND)
  INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})
ENDIF()

enable_testing()

MACRO(ADD_TESTS)
  FOREACH( loop_var ${ARGV} )
    SET_SOURCE_FILES_PROPERTIES( ${loop_var}_test.cc COMPILE_FLAGS "-DBOOST_TEST_DYN_LINK -DBOOST_TEST_MAIN -DBOOST_AUTO_TEST_MAIN=\"\" " )
    ADD_EXECUTABLE( ${loop_var}_test ${loop_var}_test.cc )
    TARGET_LINK_LIBRARIES( ${loop_var}_test ${ZYPP_LIBRARY} boost_unit_test_framework zypper_lib zypper_test_utils)
    ADD_TEST( ${loop_var}_test ${CMAKE_CURRENT_BINARY_DIR}/${loop_var}_test --catch_system_errors=no)
  endforeach()
ENDMACRO(ADD_TESTS)

ADD_SUBDIRECTORY( src )
ADD_SUBDIRECTORY( doc )
IF ( ENABLE_BUILD_TESTS )
  ADD_SUBDIRECTORY( tests )
else()
  ADD_SUBDIRECTORY( tests EXCLUDE_FROM_ALL )
endif()

IF ( ENABLE_BUILD_TRANS )
  ADD_SUBDIRECTORY( po )
else()
  ADD_SUBDIRECTORY( po EXCLUDE_FROM_ALL )
endif()

# documentation too important to be hidden in doc:
INSTALL(
  FILES HACKING
  DESTINATION ${INSTALL_PREFIX}/share/doc/packages/${PACKAGE}
)

# system-wide zypper.conf
INSTALL(
  FILES zypper.conf
  DESTINATION ${SYSCONFDIR}/zypp
)

# zypper-log tool
INSTALL(
  PROGRAMS tools/zypper-log
  DESTINATION ${INSTALL_PREFIX}/sbin
)

# needs-restarting tool
INSTALL(
  PROGRAMS tools/needs-restarting
  DESTINATION ${INSTALL_PREFIX}/bin
)

# zypper-aptitude compat tool
INSTALL(
  PROGRAMS tools/aptitude tools/apt-get tools/apt
  DESTINATION ${INSTALL_PREFIX}/bin
)
INSTALL(
  DIRECTORY tools/apt-packagemap.d/
  DESTINATION ${SYSCONFDIR}/zypp/apt-packagemap.d
)

