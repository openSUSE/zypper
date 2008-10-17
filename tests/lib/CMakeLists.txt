
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/vendor/shttpd )

ADD_LIBRARY(zypp_test_utils
 TestSetup.h
 WebServer.h
 WebServer.cc
)

TARGET_LINK_LIBRARIES(zypp_test_utils shttp zypp boost_thread-mt)
