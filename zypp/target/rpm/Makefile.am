## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS = 

INCLUDES = 						\
	-DZYPP_BASE_LOGGER_LOGGROUP=\"rpm\"

## ##################################################

targetrpmincludedir = $(pkgincludedir)/target/rpm

targetrpminclude_HEADERS = 		\
	librpm.h		\
	RpmException.h		\
	BinHeader.h		\
	RpmHeader.h		\
	librpmDb.h		\
	RpmCallbacks.h		\
	RpmPackageImpl.h	\
	RpmDb.h


noinst_LTLIBRARIES =	lib@PACKAGE@_target_rpm.la

## ##################################################

lib@PACKAGE@_target_rpm_la_SOURCES = \
	RpmException.cc		\
	BinHeader.cc		\
	RpmHeader.cc		\
	librpmDb.cc		\
	librpmDb.cv3.cc		\
	RpmCallbacks.cc		\
	RpmPackageImpl.cc	\
	RpmDb.cc


lib@PACKAGE@_target_rpm_la_LIBADD =	-lrpm

## ##################################################
