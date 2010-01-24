ADD_LIBRARY(zypp_test_utils
 TestSetup.h
)

SET_TARGET_PROPERTIES(zypp_test_utils PROPERTIES LINKER_LANGUAGE CXX)
TARGET_LINK_LIBRARIES(zypp_test_utils ${ZYPP_LIBRARY} boost_thread-mt)
