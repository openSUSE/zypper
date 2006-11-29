####################################################################
# SUBDIRECTORIES                                                   #
####################################################################

ADD_SUBDIRECTORY( plaindir ) 
ADD_SUBDIRECTORY( susetags ) 
ADD_SUBDIRECTORY( yum ) 

####################################################################
# INCLUDES                                                         #
####################################################################

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

####################################################################
# SOURCES                                                          #
####################################################################

ADD_DEFINITIONS(-DZYPP_BASE_LOGGER_LOGGROUP=\\\"source\\\")

SET(zypp_source_SRCS
SourceImpl.cc
OnMediaLocation.cc
SourceProvideFile.cc
PackageProvider.cc
PackageDelta.cc
Applydeltarpm.cc
MediaSet.cc
SourceInfo.cc
)
ADD_LIBRARY(zypp_source STATIC ${zypp_source_SRCS})
TARGET_LINK_LIBRARIES(zypp_source zypp_source_yum)
TARGET_LINK_LIBRARIES(zypp_source zypp_source_susetags)
TARGET_LINK_LIBRARIES(zypp_source zypp_source_plaindir)

####################################################################
# INSTALL                                                          #
####################################################################

INSTALL_FILES(  FILES  
SourceImpl.h
Builtin.h
OnMediaLocation.h
ManagedFile.h
SourceProvideFile.h
PackageProvider.h
PackageDelta.h
MediaSet.h
SourceInfo.h
Applydeltarpm.h
MediaSet.h
DESTINATION
${CMAKE_INSTALL_PREFIX}/include/zypp/source
)

