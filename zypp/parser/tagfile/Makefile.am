## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS =

INCLUDES =

## ##################################################

parsertagfileincludedir = $(pkgincludedir)/parser/tagfile

parsertagfileinclude_HEADERS = \
	ParseException.h\
	Tags.h		\
	Grammar.h	\
	TagFileParser.h \
  ProductMetadataParser.h	\
	MediaMetadataParser.h \
	MediaPatchesMetadataParser.h

noinst_LTLIBRARIES =	lib@PACKAGE@_parser_tagfile.la

## ##################################################

lib@PACKAGE@_parser_tagfile_la_SOURCES = \
	ParseException.cc\
	Tags.cc		\
	TagFileParser.cc \
	ProductMetadataParser.cc \
	MediaMetadataParser.cc \
	MediaPatchesMetadataParser.cc

lib@PACKAGE@_parser_tagfile_la_LIBADD = 

## ##################################################
