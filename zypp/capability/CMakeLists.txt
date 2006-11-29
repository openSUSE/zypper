INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

SET(zypp_capability_SRCS
CapabilityImpl.cc
CapTraits.cc
FileCap.cc
HalCap.cc
ModaliasCap.cc
NamedCap.cc
NullCap.cc
SplitCap.cc
VersionedCap.cc
)
ADD_LIBRARY(zypp_capability STATIC ${zypp_capability_SRCS})

SET_SOURCE_FILES_PROPERTIES(${zypp_capability_SRCS} PROPERTIES COMPILE_FLAGS -DZYPP_BASE_LOGGER_LOGGROUP=\\\"capability\\\")

INSTALL_FILES( FILES
Capabilities.h
CapabilityImpl.h
CapTraits.h
ConditionalCap.h
FileCap.h
HalCap.h
ModaliasCap.h
NamedCap.h
NullCap.h
OrCap.h
SplitCap.h
VersionedCap.h
DESTINATION
${CMAKE_INSTALL_PREFIX}/include/zypp/capability
)
