## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS =

INCLUDES =

## ##################################################

parsertagfileincludedir = $(pkgincludedir)/parser/tagfile

parsertagfileinclude_HEADERS = \
	Parser.h	\
	ParseException.h\
	Tags.h		\
	Grammar.h	\
	TagFileParser.h

noinst_LTLIBRARIES =	lib@PACKAGE@_parser_tagfile.la

## ##################################################

lib@PACKAGE@_parser_tagfile_la_SOURCES = \
	Parser.cc	\
	ParseException.cc\
	Tags.cc		\
	TagFileParser.cc

lib@PACKAGE@_parser_tagfile_la_LIBADD = 

## ##################################################
