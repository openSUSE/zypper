## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS = susetags yum

## ##################################################

sourceincludedir = $(pkgincludedir)/source

sourceinclude_HEADERS =	\
	SourceImpl.h	\
	Builtin.h	\
	MediaSet.h

noinst_LTLIBRARIES =	lib@PACKAGE@_source.la

## ##################################################

lib@PACKAGE@_source_la_SOURCES =	\
	SourceImpl.cc


lib@PACKAGE@_source_la_LIBADD = \
		susetags/lib@PACKAGE@_source_susetags.la	\
		yum/lib@PACKAGE@_source_yum.la

## ##################################################
