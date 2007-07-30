ADD_SUBDIRECTORY( yum )
ADD_SUBDIRECTORY( susetags )

ADD_EXECUTABLE( RepoVariables RepoVariables_test.cc )
TARGET_LINK_LIBRARIES( RepoVariables zypp boost_unit_test_framework )

ADD_TEST(RepoVariables ${CMAKE_CURRENT_BINARY_DIR}/RepoVariables ${CMAKE_CURRENT_SOURCE_DIR}/data )