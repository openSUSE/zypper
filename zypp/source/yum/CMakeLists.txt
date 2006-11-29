INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

SET(zypp_source_yum_SRCS
YUMSourceImpl.cc
YUMScriptImpl.cc
YUMAtomImpl.cc
YUMMessageImpl.cc
YUMPackageImpl.cc
YUMPatchImpl.cc
YUMGroupImpl.cc
YUMPatternImpl.cc
YUMProductImpl.cc
)
ADD_LIBRARY(zypp_source_yum STATIC ${zypp_source_yum_SRCS})

SET_SOURCE_FILES_PROPERTIES(${zypp_source_yum_SRCS} PROPERTIES COMPILE_FLAGS -DZYPP_BASE_LOGGER_LOGGROUP=\\\"yum\\\")

INSTALL_FILES( FILES
YUMAtomImpl.h
YUMGroupImpl.h
YUMMessageImpl.h
YUMPackageImpl.h
YUMPatchImpl.h
YUMPatternImpl.h
YUMProductImpl.h
YUMScriptImpl.h
YUMSourceImpl.h
DESTINATION
${CMAKE_INSTALL_PREFIX}/include/zypp/source/yum
)