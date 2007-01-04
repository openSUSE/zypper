INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

########### next target ###############

SET(zypp2_source_yum_SRCS
YUMSourceCacher.cc
)

ADD_LIBRARY(zypp2_source_yum STATIC ${zypp2_source_yum_SRCS})
