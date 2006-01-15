## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS = hal rpm

## ##################################################

include_HEADERS =

## ##################################################

noinst_LTLIBRARIES =	lib@PACKAGE@_target.la

## ##################################################

lib@PACKAGE@_target_la_SOURCES =

lib@PACKAGE@_target_la_LDFLAGS = @LIBZYPP_VERSION_INFO@

lib@PACKAGE@_target_la_LIBADD = \
	rpm/lib@PACKAGE@_target_rpm.la	\
	hal/lib@PACKAGE@_target_hal.la

## ##################################################
