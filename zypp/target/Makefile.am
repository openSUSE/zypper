## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS = hal rpm store

## ##################################################

targetincludedir = $(pkgincludedir)/target

targetinclude_HEADERS =			\
	TargetImpl.h

## ##################################################

noinst_LTLIBRARIES =	lib@PACKAGE@_target.la

## ##################################################

lib@PACKAGE@_target_la_SOURCES =	\
	TargetImpl.cc

lib@PACKAGE@_target_la_LDFLAGS = @LIBZYPP_VERSION_INFO@

lib@PACKAGE@_target_la_LIBADD = \
	rpm/lib@PACKAGE@_target_rpm.la	\
	hal/lib@PACKAGE@_target_hal.la \
	store/lib@PACKAGE@_target_store.la

## ##################################################
