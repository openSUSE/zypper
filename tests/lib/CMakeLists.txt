
INCLUDE_DIRECTORIES( ${LIBZYPP_SOURCE_DIR}/vendor/mongoose )

ADD_LIBRARY(zypp_test_utils
 TestSetup.h
 WebServer.h
 WebServer.cc
)

TARGET_LINK_LIBRARIES(zypp_test_utils mongoose zypp ${Boost_thread_LIBRARY})
