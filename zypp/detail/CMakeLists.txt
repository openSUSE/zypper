INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

SET(zypp_detail_SRCS
ResolvableImpl.cc
ResObjectImplIf.cc
PackageImplIf.cc
AtomImplIf.cc
SelectionImplIf.cc
PatternImplIf.cc
MessageImplIf.cc
ScriptImplIf.cc
PatchImplIf.cc
ProductImplIf.cc
SrcPackageImplIf.cc
PackageImpl.cc
SrcPackageImpl.cc
SelectionImpl.cc
PatternImpl.cc
MessageImpl.cc
ScriptImpl.cc
AtomImpl.cc
PatchImpl.cc
ProductImpl.cc
)
ADD_LIBRARY(zypp_detail STATIC ${zypp_detail_SRCS})

SET_SOURCE_FILES_PROPERTIES(${zypp_detail_SRCS} PROPERTIES COMPILE_FLAGS -DZYPP_BASE_LOGGER_LOGGROUP=\\\"detal\\\")

INSTALL_FILES( FILES
AtomImpl.h
AtomImplIf.h
ImplConnect.h
MessageImpl.h
MessageImplIf.h
PackageImpl.h
PackageImplIf.h
PatchImpl.h
PatchImplIf.h
PatternImpl.h
PatternImplIf.h
ProductImpl.h
ProductImplIf.h
ResImplTraits.h
ResObjectFactory.h
ResObjectImplIf.h
ResolvableImpl.h
ScriptImpl.h
ScriptImplIf.h
SelectionImpl.h
SelectionImplIf.h
SrcPackageImpl.h
SrcPackageImplIf.
DESTINATION
${CMAKE_INSTALL_PREFIX}/include/zypp/detail
)
