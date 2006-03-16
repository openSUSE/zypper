## Process this file with automake to produce Makefile.in
## ##################################################

#SUBDIRS = schema

INCLUDES = -I$(oldincludedir)/libxml2 -I..	\
	-DZYPP_BASE_LOGGER_LOGGROUP=\"parser-xml\"

## ##################################################

parserxmlincludedir = $(pkgincludedir)/parser/xmlstore

parserxmlinclude_HEADERS = \
	XMLProductParser.h \
	XMLPatternParser.h \
	XMLResObjectParser.h	\
	XMLPatchParser.h	\
	XMLParserData.h
	

noinst_LTLIBRARIES =	lib@PACKAGE@_parser_xmlstore.la

## ##################################################

lib@PACKAGE@_parser_xmlstore_la_SOURCES = \
	XMLParserData.cc	\
	XMLPatternParser.cc	\
	XMLResObjectParser.cc	\
	XMLPatchParser.cc	\
	XMLProductParser.cc


## ##################################################
