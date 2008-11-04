ADD_DEFINITIONS(
  -DSRC_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
  -DTESTS_SRC_DIR="${LIBZYPP_SOURCE_DIR}/tests"
)

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

#ADD_EXECUTABLE( bridge ${bridge_SRCS} )
#TARGET_LINK_LIBRARIES( bridge zypp )

#ADD_EXECUTABLE( deltarpm deltarpm.cc )
#TARGET_LINK_LIBRARIES( deltarpm zypp )

#ADD_EXECUTABLE( pathinfo pathinfo.cc )
#TARGET_LINK_LIBRARIES( pathinfo zypp )

ADD_EXECUTABLE( poolquery poolquery.cc )
TARGET_LINK_LIBRARIES( poolquery zypp )

ADD_EXECUTABLE( repos repos.cc )
TARGET_LINK_LIBRARIES( repos zypp )

#ADD_EXECUTABLE( yumparsertest YUMParser_test.cc )
#TARGET_LINK_LIBRARIES( yumparsertest zypp )

ADD_EXECUTABLE( play play.cc )
TARGET_LINK_LIBRARIES( play zypp boost_signals )
