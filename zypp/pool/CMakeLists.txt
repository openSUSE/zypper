INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )


########### next target ###############

SET(zypp_pool_SRCS
PoolImpl.cc
PoolStats.cc
GetResolvablesToInsDel.cc
)
ADD_LIBRARY(zypp_pool STATIC ${zypp_pool_SRCS})

SET_SOURCE_FILES_PROPERTIES(${zypp_pool_SRCS} PROPERTIES COMPILE_FLAGS -DZYPP_BASE_LOGGER_LOGGROUP=\\\"pool\\\")

INSTALL_FILES( FILES
GetResolvablesToInsDel.h
PoolImpl.h
PoolStats.h
PoolTraits.h
DESTINATION
${CMAKE_INSTALL_PREFIX}/include/zypp/pool
)
