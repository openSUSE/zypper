## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS =

## ##################################################

sourceincludedir = $(pkgincludedir)/source

sourceinclude_HEADERS =	\
	PackagesParser.h\
	SuseTagsImpl.h \
	SuseTagsSelectionImpl.h \
	SuseTagsPackageImpl.h \
	SelectionSelFileParser.h \
	ProductMetadataParser.h \
	MediaMetadataParser.h \
	MediaPatchesMetadataParser.h

noinst_LTLIBRARIES =	lib@PACKAGE@_source_susetags.la

## ##################################################

lib@PACKAGE@_source_susetags_la_SOURCES = \
	PackagesParser.cc	\
	SuseTagsImpl.cc \
	SuseTagsSelectionImpl.cc \
	SuseTagsPackageImpl.cc \
	SelectionSelFileParser.cc \
	ProductMetadataParser.cc \
	MediaMetadataParser.cc \
	MediaPatchesMetadataParser.cc

lib@PACKAGE@_source_susetags_la_LIBADD =

## ##################################################
