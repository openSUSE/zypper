find_program( NGINX nginx )
find_library( LIBFCGI NAMES fcgi )
find_library( LIBFCGI++ NAMES fcgi++ )

if ( NOT NGINX )
  message( FATAL_ERROR "nginx not found" )
else()
  message ( "Using nginx binary at: ${NGINX}" )
  ADD_DEFINITIONS( -DWEBSRV_BINARY="${NGINX}" )
endif()

if ( NOT LIBFCGI )
  message( FATAL_ERROR "libfcgi not found" )
else()
  message ( "Using libfcgi library at: ${LIBFCGI}" )
endif()

if ( NOT LIBFCGI++ )
  message( FATAL_ERROR "libfcgi++ not found" )
else()
  message ( "Using libfcgi++ library at: ${LIBFCGI++}" )
endif()

add_definitions( -DSRCDIR="${CMAKE_CURRENT_SOURCE_DIR}")

ADD_LIBRARY(zypp_test_utils
 TestSetup.h
 TestTools.h
 WebServer.h
 WebServer.cc
)

TARGET_LINK_LIBRARIES( zypp_test_utils zypp-allsym ${LIBFCGI} ${LIBFCGI++} pthread )
