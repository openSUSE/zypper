ADD_DEFINITIONS(-DSRC_DIR=\\\"${CMAKE_CURRENT_SOURCE_DIR}\\\")

#ADD_EXECUTABLE(sourcedownload sourcedownload.cc)
#TARGET_LINK_LIBRARIES(sourcedownload zypp zypp2)



SET( bridge_SRCS
  bridge/bridge.cc
  bridge/BaseImpl.cc
  bridge/Derived.cc
)

SET( bridge_HEADERS
  bridge/Base.h
  bridge/BaseImpl.h
  bridge/Derived.h
)

ADD_EXECUTABLE( yumparsertest YUMParser_test.cc )
TARGET_LINK_LIBRARIES(yumparsertest zypp zypp2)

ADD_EXECUTABLE( bridge ${bridge_SRCS} )
TARGET_LINK_LIBRARIES( bridge zypp zypp2)
