ADD_SUBDIRECTORY( yum ) 

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )


########### next target ###############

SET(Msg_SRCS
Msg.cc
)

ADD_EXECUTABLE(Msg ${Msg_SRCS})


SET(Msg.debug_SRCS
$(Msg_SOURCES)
)

ADD_EXECUTABLE(Msg.debug ${Msg.debug_SRCS})

SET(Patch_SRCS
Patch.cc
)

ADD_EXECUTABLE(Patch ${Patch_SRCS})

SET(Patch.debug_SRCS
$(Patch_SOURCES)
)

ADD_EXECUTABLE(Patch.debug ${Patch.debug_SRCS})
