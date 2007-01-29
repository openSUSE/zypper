ADD_SUBDIRECTORY( hal ) 
ADD_SUBDIRECTORY( modalias ) 
ADD_SUBDIRECTORY( rpm ) 
ADD_SUBDIRECTORY( store ) 

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

SET(zypp_target_SRCS
TargetImpl.cc
TargetCallbackReceiver.cc
TargetException.cc
CommitLog.cc
CommitPackageCache.cc
CommitPackageCacheImpl.cc
CommitPackageCacheReadAhead.cc
)

ADD_LIBRARY(zypp_target STATIC ${zypp_target_SRCS})
TARGET_LINK_LIBRARIES(zypp_target zypp_target_hal)
TARGET_LINK_LIBRARIES(zypp_target zypp_target_modalias)
TARGET_LINK_LIBRARIES(zypp_target zypp_target_rpm)
TARGET_LINK_LIBRARIES(zypp_target zypp_target_store)

SET_SOURCE_FILES_PROPERTIES(${zypp_target_SRCS} PROPERTIES COMPILE_FLAGS -DZYPP_BASE_LOGGER_LOGGROUP=\\\"target\\\")

INSTALL_FILES( FILES
CommitLog.h
TargetCallbackReceiver.h
TargetException.h
TargetImpl.h
CommitPackageCache.h
CommitPackageCacheReadAhead.h
DESTINATION
${CMAKE_INSTALL_PREFIX}/include/zypp/target
)
