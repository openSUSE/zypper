ADD_SUBDIRECTORY( yum ) 
ADD_SUBDIRECTORY( sqlite-source )

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )


########### next target ###############

SET(zypp2_source_SRCS
dummy.cc
)

ADD_LIBRARY(zypp2_source STATIC ${zypp2_source_SRCS})
TARGET_LINK_LIBRARIES(zypp2_source zypp2_source_yum)
TARGET_LINK_LIBRARIES(zypp2_source zypp2_source_sqlite)
