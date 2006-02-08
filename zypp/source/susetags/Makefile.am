## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS =

## ##################################################

sourceincludedir = $(pkgincludedir)/source

sourceinclude_HEADERS =	\
	PackagesParser.h\
	PackagesLangParser.h\
	SuseTagsImpl.h \
	SuseTagsSelectionImpl.h \
	SuseTagsPatternImpl.h \
	SuseTagsPackageImpl.h \
	SelectionTagFileParser.h \
	PatternTagFileParser.h \
	ProductMetadataParser.h \
	MediaMetadataParser.h \
	MediaPatchesMetadataParser.h

noinst_LTLIBRARIES =	lib@PACKAGE@_source_susetags.la

## ##################################################

lib@PACKAGE@_source_susetags_la_SOURCES = \
	PackagesParser.cc	\
	PackagesLangParser.cc	\
	SuseTagsImpl.cc \
	SuseTagsSelectionImpl.cc \
	SuseTagsPatternImpl.cc \
	SuseTagsPackageImpl.cc \
	SelectionTagFileParser.cc \
	PatternTagFileParser.cc \
	ProductMetadataParser.cc \
	MediaMetadataParser.cc \
	MediaPatchesMetadataParser.cc

lib@PACKAGE@_source_susetags_la_LIBADD =

## ##################################################
