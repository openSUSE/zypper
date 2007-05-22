INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

ADD_DEFINITIONS(-DSRC_DIR=\\\"${CMAKE_CURRENT_SOURCE_DIR}\\\")


ADD_EXECUTABLE(mediaaccess mediaaccess.cc)
TARGET_LINK_LIBRARIES(mediaaccess  zypp )

ADD_EXECUTABLE(scansource ScanSource.cc)
TARGET_LINK_LIBRARIES(scansource  zypp )

ADD_EXECUTABLE(susetags-downloader SUSETagsDownloader_tp.cc)
TARGET_LINK_LIBRARIES(susetags-downloader  zypp )

ADD_EXECUTABLE(cachestore CacheStore_tp.cc)
TARGET_LINK_LIBRARIES(cachestore  zypp )
TARGET_LINK_LIBRARIES(cachestore  zypp2 )

ADD_EXECUTABLE(testbed testbed.cc)
TARGET_LINK_LIBRARIES(testbed  zypp )
TARGET_LINK_LIBRARIES(testbed  zypp2 )

ADD_EXECUTABLE(yum-downloader YUMDownloader_tp.cc)
TARGET_LINK_LIBRARIES(yum-downloader  zypp )
TARGET_LINK_LIBRARIES(yum-downloader  zypp2 )

ADD_EXECUTABLE(yum-reader YUMReader_tp.cc)
TARGET_LINK_LIBRARIES(yum-reader  zypp )
TARGET_LINK_LIBRARIES(yum-reader  zypp2 )

ADD_EXECUTABLE(cachedsource CachedSource_tp.cc)
TARGET_LINK_LIBRARIES(cachedsource  zypp )
TARGET_LINK_LIBRARIES(cachedsource  zypp2 )

ADD_EXECUTABLE(repomanager RepositoryManager_tp.cc)
TARGET_LINK_LIBRARIES(repomanager zypp )
TARGET_LINK_LIBRARIES(repomanager zypp2 )

FIND_PACKAGE(Zsync)
IF(ZSYNC_FOUND)
  ADD_EXECUTABLE(zsync zsync.cc)
  TARGET_LINK_LIBRARIES(zsync  ${ZSYNC_LIBRARY} ${RCKSUM_LIBRARY})
  TARGET_LINK_LIBRARIES(zsync  zypp )
#  TARGET_LINK_LIBRARIES(zsync  zypp2 )
ENDIF(ZSYNC_FOUND)