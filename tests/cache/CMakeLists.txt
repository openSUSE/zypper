INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

ADD_EXECUTABLE(CacheInitializer CacheInitializer_test.cc)
TARGET_LINK_LIBRARIES(CacheInitializer boost_unit_test_framework )
TARGET_LINK_LIBRARIES(CacheInitializer zypp2 )
TARGET_LINK_LIBRARIES(CacheInitializer zypp )

ADD_EXECUTABLE(CacheStore CacheStore_test.cc)
TARGET_LINK_LIBRARIES(CacheStore boost_unit_test_framework )
TARGET_LINK_LIBRARIES(CacheStore zypp2 )
TARGET_LINK_LIBRARIES(CacheStore zypp )

ADD_TEST(CacheInitializer ${CMAKE_CURRENT_BINARY_DIR}/CacheInitializer ${CMAKE_CURRENT_SOURCE_DIR}/data )
ADD_TEST(CacheStore ${CMAKE_CURRENT_BINARY_DIR}/CacheStore ${CMAKE_CURRENT_SOURCE_DIR}/data )

