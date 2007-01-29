SUBDIRS = sqlite3x

INCLUDES = -DZYPP_BASE_LOGGER_LOGGROUP=\"cache\"

## ##################################################

cacheincludedir = $(pkgincludedir)/cache

cacheinclude_HEADERS = SourceCache.h CacheInitializer.h SourceCacher.h KnownSourcesCache.h Utils.h

## ##################################################

noinst_LTLIBRARIES =	lib@PACKAGE@2_cache.la

## ##################################################

lib@PACKAGE@2_cache_la_SOURCES =	SourceCache.cpp CacheInitializer.cpp SourceCacher.cpp KnownSourcesCache.cpp Utils.cpp

lib@PACKAGE@2_cache_la_LDFLAGS = @LIBZYPP_VERSION_INFO@

lib@PACKAGE@2_cache_la_LIBADD = sqlite3x/lib@PACKAGE@2_sqlite3x.la
##	hal/lib@PACKAGE@_cache_hal.la \
##	modalias/lib@PACKAGE@_cache_modalias.la \
##	store/lib@PACKAGE@_cache_store.la

