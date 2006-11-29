INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

SET(zypp_source_susetags_SRCS
PackagesParser.cc
PackagesLangParser.cc
SuseTagsImpl.cc
SuseTagsSelectionImpl.cc
SuseTagsPatternImpl.cc
SuseTagsPackageImpl.cc
SuseTagsSrcPackageImpl.cc
SuseTagsProductImpl.cc
SelectionTagFileParser.cc
PatternTagFileParser.cc
ProductMetadataParser.cc
MediaMetadataParser.cc
MediaPatchesMetadataParser.cc
)
ADD_LIBRARY(zypp_source_susetags STATIC ${zypp_source_susetags_SRCS})

SET_SOURCE_FILES_PROPERTIES(${zypp_source_susetags_SRCS} PROPERTIES COMPILE_FLAGS -DZYPP_BASE_LOGGER_LOGGROUP=\\\"susetags\\\")

INSTALL_FILES( FILES
MediaMetadataParser.h
MediaPatchesMetadataParser.h
PackagesLangParser.h
PackagesParser.h
PatternTagFileParser.h
ProductMetadataParser.h
SelectionTagFileParser.h
SuseTagsImpl.h
SuseTagsPackageImpl.h
SuseTagsPatternImpl.h
SuseTagsProductImpl.h
SuseTagsSelectionImpl.h
SuseTagsSrcPackageImpl.h
DESTINATION
${CMAKE_INSTALL_PREFIX}/include/zypp/source/susetags
)