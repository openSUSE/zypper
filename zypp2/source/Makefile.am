## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS = yum sqlite-source

INCLUDES = 					\
	-DZYPP_BASE_LOGGER_LOGGROUP=\"source2\"

## ##################################################

sourceincludedir = $(pkgincludedir)/source

sourceinclude_HEADERS =

noinst_LTLIBRARIES =	lib@PACKAGE@2_source.la

## ##################################################

lib@PACKAGE@2_source_la_SOURCES = dummy.cc

lib@PACKAGE@2_source_la_LIBADD = \
		yum/lib@PACKAGE@2_source_yum.la
    
## ##################################################
