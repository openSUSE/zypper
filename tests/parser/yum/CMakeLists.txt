ADD_EXECUTABLE( RepomdFileReader RepomdFileReader_test.cc )
TARGET_LINK_LIBRARIES( RepomdFileReader zypp boost_unit_test_framework )

ADD_TEST(RepomdFileReader ${CMAKE_CURRENT_BINARY_DIR}/RepomdFileReader ${CMAKE_CURRENT_SOURCE_DIR}/data )