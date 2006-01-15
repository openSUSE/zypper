## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS = schema

INCLUDES = -I$(oldincludedir)/libxml2 -I..

## ##################################################

parseryumincludedir = $(pkgincludedir)/parser/yum

parseryuminclude_HEADERS = \
	YUMFileListParser.h	\
	YUMOtherParser.h	\
	YUMParser.h		\
	YUMPrimaryParser.h	\
	YUMGroupParser.h	\
	YUMParserData.h		\
	YUMPatchParser.h	\
	YUMPatchesParser.h	\
	YUMProductParser.h	\
	YUMRepomdParser.h	\
	schemanames.h


noinst_LTLIBRARIES =	lib@PACKAGE@_parser_yum.la

## ##################################################

lib@PACKAGE@_parser_yum_la_SOURCES = \
	YUMParserData.cc	\
	YUMFileListParser.cc	\
	YUMRepomdParser.cc	\
	YUMGroupParser.cc	\
	YUMPatchParser.cc	\
	YUMPatchesParser.cc	\
	YUMOtherParser.cc	\
	YUMPrimaryParser.cc	\
	YUMProductParser.cc


## ##################################################
