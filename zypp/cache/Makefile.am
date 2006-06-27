SUBDIRS = sqlite3x

INCLUDES = -DZYPP_BASE_LOGGER_LOGGROUP=\"cache\"

## ##################################################

cacheincludedir = $(pkgincludedir)/cache

cacheinclude_HEADERS = KnownSourcesCache.h

## ##################################################

noinst_LTLIBRARIES =	lib@PACKAGE@_cache.la

## ##################################################

lib@PACKAGE@_cache_la_SOURCES =	KnownSourcesCache.cpp

lib@PACKAGE@_cache_la_LDFLAGS = @LIBZYPP_VERSION_INFO@

lib@PACKAGE@_cache_la_LIBADD = sqlite3x/lib@PACKAGE@_sqlite3x.la
##	hal/lib@PACKAGE@_cache_hal.la \
##	modalias/lib@PACKAGE@_cache_modalias.la \
##	store/lib@PACKAGE@_cache_store.la

