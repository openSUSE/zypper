INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )


########### next target ###############

SET(read_plaindir_source_SRCS
read_plaindir_source.cc
)

ADD_EXECUTABLE(read_plaindir_source ${read_plaindir_source_SRCS})
