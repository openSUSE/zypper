INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

SET( querypool_SRC
zypp-query-pool.cc
)

ADD_EXECUTABLE( zypp-query-pool ${querypool_SRC} )
TARGET_LINK_LIBRARIES( zypp-query-pool zypp )
TARGET_LINK_LIBRARIES( zypp-query-pool xml2 )

INSTALL(TARGETS zypp-query-pool RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/zypp )