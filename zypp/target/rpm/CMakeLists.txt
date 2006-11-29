INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )


########### next target ###############

SET(zypp_target_rpm_SRCS
RpmException.cc
BinHeader.cc
RpmHeader.cc
librpmDb.cc
librpmDb.cv3.cc
RpmCallbacks.cc
RpmPackageImpl.cc
RpmDb.cc
)
ADD_LIBRARY(zypp_target_rpm STATIC ${zypp_target_rpm_SRCS})

SET_SOURCE_FILES_PROPERTIES(${zypp_target_rpm_SRCS} PROPERTIES COMPILE_FLAGS -DZYPP_BASE_LOGGER_LOGGROUP=\\\"rpm\\\")

INSTALL_FILES( FILES
BinHeader.h
librpmDb.h
librpm.h
RpmCallbacks.h
RpmDb.h
RpmException.h
RpmHeader.h
RpmPackageImpl.h
DESTINATION
${CMAKE_INSTALL_PREFIX}/include/zypp/target/rpm
)
