INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

SET(CacheInitializer_SRCS
CacheInitializer.cc
)

ADD_EXECUTABLE(CacheInitializer ${CacheInitializer_SRCS})
TARGET_LINK_LIBRARIES(CacheInitializer boost_unit_test_framework )
TARGET_LINK_LIBRARIES(CacheInitializer zypp2 )
TARGET_LINK_LIBRARIES(CacheInitializer zypp )
