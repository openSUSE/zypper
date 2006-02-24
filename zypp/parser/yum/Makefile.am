## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS = schema

INCLUDES = -I$(oldincludedir)/libxml2 -I..	\
	-DZYPP_BASE_LOGGER_LOGGROUP=\"parser-yum\"

## ##################################################

parseryumincludedir = $(pkgincludedir)/parser/yum

parseryuminclude_HEADERS = \
	YUMFileListParser.h	\
	YUMOtherParser.h	\
	YUMParser.h		\
	YUMPrimaryParser.h	\
	YUMGroupParser.h	\
	YUMPatternParser.h	\
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
	YUMPatternParser.cc	\
	YUMPatchParser.cc	\
	YUMPatchesParser.cc	\
	YUMOtherParser.cc	\
	YUMPrimaryParser.cc	\
	YUMProductParser.cc


## ##################################################
