INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )


########### next target ###############

SET(zypp_thread_SRCS
Mutex.cc
)
ADD_LIBRARY(zypp_thread STATIC ${zypp_thread_SRCS})

SET_SOURCE_FILES_PROPERTIES(${zypp_thread_SRCS} PROPERTIES COMPILE_FLAGS -DZYPP_BASE_LOGGER_LOGGROUP=\\\"thread\\\")

INSTALL_FILES( FILES
MutexException.h
Mutex.h
MutexLock.h
Once.h
DESTINATION
${CMAKE_INSTALL_PREFIX}/include/zypp/thread
)