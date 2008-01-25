ADD_DEFINITIONS(-DSRC_DIR=\\\"${CMAKE_CURRENT_SOURCE_DIR}\\\")

ADD_EXECUTABLE(mediaaccess mediaaccess.cc)
TARGET_LINK_LIBRARIES(mediaaccess  zypp )

#ADD_EXECUTABLE(scansource ScanSource.cc)
#TARGET_LINK_LIBRARIES(scansource  zypp )

#ADD_EXECUTABLE(susetags-downloader SUSETagsDownloader_tp.cc)
#TARGET_LINK_LIBRARIES(susetags-downloader  zypp )

ADD_EXECUTABLE(solvstore SolvStore_tp.cc)
TARGET_LINK_LIBRARIES(solvstore zypp )

#ADD_EXECUTABLE(testbed testbed.cc)
#TARGET_LINK_LIBRARIES(testbed  zypp )

#ADD_EXECUTABLE(yum-downloader YUMDownloader_tp.cc)
#TARGET_LINK_LIBRARIES(yum-downloader  zypp )

# ADD_EXECUTABLE(yum-reader YUMReader_tp.cc)
# TARGET_LINK_LIBRARIES(yum-reader  zypp )

ADD_EXECUTABLE(cachedsource CachedSource_tp.cc)
TARGET_LINK_LIBRARIES(cachedsource  zypp )

ADD_EXECUTABLE(repomanager RepositoryManager_tp.cc)
TARGET_LINK_LIBRARIES(repomanager zypp )

FIND_PACKAGE(Zsync)
IF(ZSYNC_FOUND)
  ADD_EXECUTABLE(zsync zsync.cc)
  TARGET_LINK_LIBRARIES(zsync  ${ZSYNC_LIBRARY} ${RCKSUM_LIBRARY})
  TARGET_LINK_LIBRARIES(zsync  zypp )
#  TARGET_LINK_LIBRARIES(zsync  zypp2 )
ENDIF(ZSYNC_FOUND)