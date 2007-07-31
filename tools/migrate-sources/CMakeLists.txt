SET( migrate_SRCS
migrate-sources.cc
)

ADD_EXECUTABLE( migrate-sources ${migrate_SRCS} )
TARGET_LINK_LIBRARIES( migrate-sources zypp )
TARGET_LINK_LIBRARIES( migrate-sources xml2 )