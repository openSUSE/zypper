add_definitions( -DSRCDIR="${CMAKE_CURRENT_SOURCE_DIR}")

ADD_LIBRARY(zypp_test_utils
 TestSetup.h
 WebServer.h
 WebServer.cc
)

TARGET_LINK_LIBRARIES( zypp_test_utils zypp-allsym fcgi fcgi++ pthread )
