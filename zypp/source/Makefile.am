## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS = plaindir susetags yum

INCLUDES = 					\
	-DZYPP_BASE_LOGGER_LOGGROUP=\"source\"

## ##################################################

sourceincludedir = $(pkgincludedir)/source

sourceinclude_HEADERS =	\
	SourceImpl.h	\
	Builtin.h	\
	PackageDelta.h	\
	MediaSet.h

noinst_LTLIBRARIES =	lib@PACKAGE@_source.la

## ##################################################

lib@PACKAGE@_source_la_SOURCES = \
	SourceImpl.cc	\
	PackageDelta.cc	\
	MediaSet.cc


lib@PACKAGE@_source_la_LIBADD = \
		plaindir/lib@PACKAGE@_source_plaindir.la	\
		susetags/lib@PACKAGE@_source_susetags.la	\
		yum/lib@PACKAGE@_source_yum.la

## ##################################################
