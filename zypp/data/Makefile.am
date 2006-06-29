## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS =

INCLUDES = -I$(oldincludedir)/libxml2	\
	-DZYPP_BASE_LOGGER_LOGGROUP=\"data\"

## ##################################################

parserincludedir = $(pkgincludedir)/data

parserinclude_HEADERS = \
	ResolvableData.h
	
noinst_LTLIBRARIES =	lib@PACKAGE@_data.la


## ##################################################

lib@PACKAGE@_data_la_SOURCES = \
	ResolvableData.cc
