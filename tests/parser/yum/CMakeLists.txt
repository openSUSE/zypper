ADD_DEFINITIONS( -DBOOST_TEST_DYN_LINK -DBOOST_TEST_MAIN )

ADD_EXECUTABLE( RepomdFileReader RepomdFileReader_test.cc )
TARGET_LINK_LIBRARIES( RepomdFileReader zypp boost_unit_test_framework )

ADD_EXECUTABLE( PatchesFileReader PatchesFileReader_test.cc )
TARGET_LINK_LIBRARIES( PatchesFileReader zypp boost_unit_test_framework )

ADD_EXECUTABLE( PatchFileReader PatchFileReader_test.cc )
TARGET_LINK_LIBRARIES( PatchFileReader zypp boost_unit_test_framework )

ADD_TEST(RepomdFileReader ${CMAKE_CURRENT_BINARY_DIR}/RepomdFileReader ${CMAKE_CURRENT_SOURCE_DIR}/data )
ADD_TEST(PatchesFileReader ${CMAKE_CURRENT_BINARY_DIR}/PatchesFileReader ${CMAKE_CURRENT_SOURCE_DIR}/data )
ADD_TEST(PatchFileReader ${CMAKE_CURRENT_BINARY_DIR}/PatchFileReader ${CMAKE_CURRENT_SOURCE_DIR}/data )
