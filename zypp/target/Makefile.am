## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS = hal modalias rpm store

INCLUDES = -DZYPP_BASE_LOGGER_LOGGROUP=\"target\"

## ##################################################

targetincludedir = $(pkgincludedir)/target

targetinclude_HEADERS =			\
	TargetImpl.h			\
	TargetCallbackReceiver.h	\
	TargetException.h

## ##################################################

noinst_LTLIBRARIES =	lib@PACKAGE@_target.la

## ##################################################

lib@PACKAGE@_target_la_SOURCES =	\
	TargetImpl.cc			\
	TargetCallbackReceiver.cc	\
	TargetException.cc

lib@PACKAGE@_target_la_LDFLAGS = @LIBZYPP_VERSION_INFO@

lib@PACKAGE@_target_la_LIBADD = \
	rpm/lib@PACKAGE@_target_rpm.la	\
	hal/lib@PACKAGE@_target_hal.la \
	store/lib@PACKAGE@_target_store.la

## ##################################################
