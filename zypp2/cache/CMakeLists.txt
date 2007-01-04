INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )


ADD_SUBDIRECTORY(sqlite3x)

########### next target ###############

SET(zypp2_cache_SRCS
SourceCache.cpp
SourceCacheInitializer.cpp
SourceCacher.cpp
KnownSourcesCache.cpp
Utils.cpp
)

ADD_LIBRARY(zypp2_cache SHARED ${zypp2_cache_SRCS})
TARGET_LINK_LIBRARIES(zypp2_cache zypp2_sqlite3x )

########### install files ###############

INSTALL_FILES(  FILES  SourceCache.h SourceCacheInitializer.h SourceCacher.h KnownSourcesCache.h Utils.h )



#original Makefile.am contents follow:

#SUBDIRS = sqlite3x
#
#INCLUDES = -DZYPP_BASE_LOGGER_LOGGROUP=\"cache\"
#
### ##################################################
#
#cacheincludedir = $(pkgincludedir)/cache
#
#cacheinclude_HEADERS = SourceCache.h SourceCacheInitializer.h SourceCacher.h KnownSourcesCache.h Utils.h
#
### ##################################################
#
#noinst_LTLIBRARIES =	lib@PACKAGE@2_cache.la
#
### ##################################################
#
#lib@PACKAGE@2_cache_la_SOURCES =	SourceCache.cpp SourceCacheInitializer.cpp SourceCacher.cpp KnownSourcesCache.cpp Utils.cpp
#
#lib@PACKAGE@2_cache_la_LDFLAGS = @LIBZYPP_VERSION_INFO@
#
#lib@PACKAGE@2_cache_la_LIBADD = sqlite3x/lib@PACKAGE@2_sqlite3x.la
###	hal/lib@PACKAGE@_cache_hal.la \
###	modalias/lib@PACKAGE@_cache_modalias.la \
###	store/lib@PACKAGE@_cache_store.la
#
