INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )


########### next target ###############

SET(test_SRCS
zypp-keyring.cc
)

ADD_EXECUTABLE(test ${test_SRCS})

TARGET_LINK_LIBRARIES(test  zypp )

########### next target ###############

SET(lock_SRCS
zypp-lock.cc
)

ADD_EXECUTABLE(lock ${lock_SRCS})
TARGET_LINK_LIBRARIES(lock  zypp )


########### next target ###############

SET(testbed_SRCS
testbed.cc
)

ADD_EXECUTABLE(testbed ${testbed_SRCS})
TARGET_LINK_LIBRARIES(testbed  zypp )


########### next target ###############

SET(aj_SRCS
aj.cc
)

ADD_EXECUTABLE(aj ${aj_SRCS})
TARGET_LINK_LIBRARIES(aj  zypp )


########### next target ###############

SET(yumparser_SRCS
yum-parser.cc
)

ADD_EXECUTABLE(yumparser ${yumparser_SRCS})
TARGET_LINK_LIBRARIES(yumparser  zypp )


########### next target ###############

SET(media_SRCS
media-glob.cc
)

ADD_EXECUTABLE(media ${media_SRCS})
TARGET_LINK_LIBRARIES(media  zypp )

