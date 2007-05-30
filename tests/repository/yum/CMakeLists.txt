ADD_EXECUTABLE( YUMDownloader YUMDownloader_test.cc )
TARGET_LINK_LIBRARIES( YUMDownloader zypp boost_unit_test_framework )

ADD_TEST(YUMDownloader ${CMAKE_CURRENT_BINARY_DIR}/YUMDownloader ${CMAKE_CURRENT_SOURCE_DIR}/data )