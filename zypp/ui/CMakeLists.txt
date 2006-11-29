INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

SET(zypp_ui_SRCS
PatchContents.cc
PatternContents.cc
Status.cc
Selectable.cc
UserWantedPackages.cc
PatchContentsImpl.cc
PatternContentsImpl.cc
PatternExpander.cc
SelectableImpl.cc
)
ADD_LIBRARY(zypp_ui STATIC ${zypp_ui_SRCS})

SET_SOURCE_FILES_PROPERTIES(${zypp_ui_SRCS} PROPERTIES COMPILE_FLAGS -DZYPP_BASE_LOGGER_LOGGROUP=\\\"ui\\\")

INSTALL_FILES( FILES
PatchContents.h
PatchContentsImpl.h
PatternContents.h
PatternContentsImpl.h
PatternExpander.h
Selectable.h
SelectableImpl.h
SelectableTraits.h
SelFilters.h
Status.h
UserWantedPackages.h
DESTINATION
${CMAKE_INSTALL_PREFIX}/include/zypp/ui
)