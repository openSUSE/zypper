## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS =

INCLUDES = -I$(oldincludedir)/libxml2 -I..	\
	-DZYPP_BASE_LOGGER_LOGGROUP=\"parser-yum2\"

## ##################################################

parseryum2includedir = $(pkgincludedir)/parser/yum2

parseryum2include_HEADERS = \
	YUMPrimaryParser.h

noinst_LTLIBRARIES =	lib@PACKAGE@_parser_yum2.la

## ##################################################

lib@PACKAGE@_parser_yum2_la_SOURCES = \
	YUMPrimaryParser.cc

## ##################################################
