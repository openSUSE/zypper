ADD_LIBRARY(zypper_test_utils
 TestSetup.h
)

SET_TARGET_PROPERTIES(zypper_test_utils PROPERTIES LINKER_LANGUAGE CXX)
TARGET_LINK_LIBRARIES(zypper_test_utils ${ZYPP_LIBRARY} boost_thread-mt)
