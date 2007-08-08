SET( migrate_SRCS
migrate-sources.cc
)

ADD_EXECUTABLE( zypp-migrate-sources ${migrate_SRCS} )
TARGET_LINK_LIBRARIES( zypp-migrate-sources zypp )
TARGET_LINK_LIBRARIES( zypp-migrate-sources xml2 )

INSTALL(TARGETS zypp-migrate-sources RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/zypp )