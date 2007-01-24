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
	OnMediaLocation.h	\
	SourceProvideFile.h	\
	PackageProvider.h	\
	PackageDelta.h	\
	MediaSet.h \
	SourceInfo.h \
	Applydeltarpm.h	\
	MediaSet.h

noinst_LTLIBRARIES =	lib@PACKAGE@_source.la

## ##################################################

lib@PACKAGE@_source_la_SOURCES = \
	SourceImpl.cc		\
	OnMediaLocation.cc	\
	SourceProvideFile.cc	\
	PackageProvider.cc	\
	PackageDelta.cc		\
	Applydeltarpm.cc	\
	MediaSet.cc \
	SourceInfo.cc



lib@PACKAGE@_source_la_LIBADD = \
		plaindir/lib@PACKAGE@_source_plaindir.la	\
		susetags/lib@PACKAGE@_source_susetags.la	\
		yum/lib@PACKAGE@_source_yum.la

## ##################################################
