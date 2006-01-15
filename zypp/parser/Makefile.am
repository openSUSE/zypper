## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS = yum tagfile

INCLUDES = -I$(oldincludedir)/libxml2

## ##################################################

parserincludedir = $(pkgincludedir)/parser

parserinclude_HEADERS = \
	XMLNodeIterator.h	\
	LibXMLHelper.h		\
	\
	Spirit.h


noinst_LTLIBRARIES =	lib@PACKAGE@_parser.la


## ##################################################

lib@PACKAGE@_parser_la_SOURCES = \
	XMLNodeIterator.cc	\
	LibXMLHelper.cc		\
	\
	Spirit.cc

lib@PACKAGE@_parser_la_LIBADD =	yum/lib@PACKAGE@_parser_yum.la	\
				-lxml2	\
				-lz	\
				tagfile/lib@PACKAGE@_parser_tagfile.la

## ##################################################
