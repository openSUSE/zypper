## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS = hal modalias rpm store

INCLUDES = -DZYPP_BASE_LOGGER_LOGGROUP=\"target\"

## ##################################################

targetincludedir = $(pkgincludedir)/target

targetinclude_HEADERS =			\
	TargetImpl.h			\
	CommitPackageCache.h		\
	CommitPackageCacheImpl.h	\
	CommitPackageCacheReadAhead.h	\
	TargetCallbackReceiver.h	\
	TargetException.h		\
	CommitLog.h

## ##################################################

noinst_LTLIBRARIES =	lib@PACKAGE@_target.la

## ##################################################

lib@PACKAGE@_target_la_SOURCES =	\
	TargetImpl.cc			\
	CommitPackageCache.cc		\
	CommitPackageCacheImpl.cc	\
	CommitPackageCacheReadAhead.cc	\
	TargetCallbackReceiver.cc	\
	TargetException.cc		\
	CommitLog.cc

lib@PACKAGE@_target_la_LDFLAGS = @LIBZYPP_VERSION_INFO@

lib@PACKAGE@_target_la_LIBADD = \
	rpm/lib@PACKAGE@_target_rpm.la	\
	hal/lib@PACKAGE@_target_hal.la \
	modalias/lib@PACKAGE@_target_modalias.la \
	store/lib@PACKAGE@_target_store.la

## ##################################################
