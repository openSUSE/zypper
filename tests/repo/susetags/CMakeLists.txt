ADD_DEFINITIONS( -DBOOST_TEST_DYN_LINK -DBOOST_TEST_MAIN )

ADD_EXECUTABLE( 
  Downloader
  Downloader_test.cc
)
TARGET_LINK_LIBRARIES( Downloader zypp boost_unit_test_framework )

ADD_TEST(Downloader ${CMAKE_CURRENT_BINARY_DIR}/Downloader ${CMAKE_CURRENT_SOURCE_DIR}/data )
