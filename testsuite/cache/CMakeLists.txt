INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

SET(SourceCacheInitializer_SRCS
SourceCacheInitializer.cc
)

ADD_EXECUTABLE(SourceCacheInitializer ${SourceCacheInitializer_SRCS})
TARGET_LINK_LIBRARIES(SourceCacheInitializer boost_unit_test_framework )
TARGET_LINK_LIBRARIES(SourceCacheInitializer zypp2 )
TARGET_LINK_LIBRARIES(SourceCacheInitializer zypp )
