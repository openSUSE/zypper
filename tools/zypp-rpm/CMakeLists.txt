PROJECT( zypp-rpm C CXX )

set (CMAKE_CXX_STANDARD 17)
SET (CMAKE_CXX_EXTENSIONS OFF)

include(FindPkgConfig)

pkg_check_modules ( SIGCPP REQUIRED sigc++-2.0 )
INCLUDE_DIRECTORIES( ${SIGCPP_INCLUDE_DIRS} )

pkg_check_modules ( LIBGLIB REQUIRED glib-2.0 )
INCLUDE_DIRECTORIES( ${LIBGLIB_INCLUDE_DIRS} )

FIND_PACKAGE(Protobuf REQUIRED)

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

SET( SOURCES
  errorcodes.h
  main.cc
  BinHeader.h
  BinHeader.cc
)

add_executable( ${PROJECT_NAME} ${SOURCES} )
target_link_libraries( ${PROJECT_NAME} zypp-core )
target_link_libraries( ${PROJECT_NAME} zypp-protobuf )
target_link_libraries( ${PROJECT_NAME} util )
target_link_libraries( ${PROJECT_NAME} ${LIBGLIB_LIBRARIES} )
target_link_libraries( ${PROJECT_NAME} ${SIGCPP_LIBRARIES} )
target_link_libraries( ${PROJECT_NAME} ${PROTOBUF_LITE_LIBRARIES} )
target_link_libraries( ${PROJECT_NAME} ${RPM_LIBRARY} ${RPMIO_LIBRARY} )

INSTALL( TARGETS ${PROJECT_NAME}	DESTINATION "${ZYPP_LIBEXEC_INSTALL_DIR}" )
