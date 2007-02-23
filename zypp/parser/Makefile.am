## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS = xml yum2 yum tagfile taggedfile xmlstore

INCLUDES = -I$(oldincludedir)/libxml2	\
	-DZYPP_BASE_LOGGER_LOGGROUP=\"parser\"

## ##################################################

parserincludedir = $(pkgincludedir)/parser

parserinclude_HEADERS = \
	XMLNodeIterator.h	\
	SAXParser.h	\
	LibXMLHelper.h		\
	xml_parser_assert.h \
	ParserProgress.h

noinst_LTLIBRARIES =	lib@PACKAGE@_parser.la


## ##################################################

lib@PACKAGE@_parser_la_SOURCES = \
	XMLNodeIterator.cc	\
	LibXMLHelper.cc \
	SAXParser.cc

lib@PACKAGE@_parser_la_LIBADD =	xml/lib@PACKAGE@_parser_xml.la	\
				yum/lib@PACKAGE@_parser_yum.la	\
				yum2/lib@PACKAGE@_parser_yum2.la	\
				-lxml2	\
				-lz	\
				tagfile/lib@PACKAGE@_parser_tagfile.la	\
				taggedfile/lib@PACKAGE@_parser_taggedfile.la	\
				xmlstore/lib@PACKAGE@_parser_xmlstore.la

## ##################################################
