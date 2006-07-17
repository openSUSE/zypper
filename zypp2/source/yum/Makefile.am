## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS =

INCLUDES = 						\
	-DZYPP_BASE_LOGGER_LOGGROUP=\"yum\"

## ##################################################

sourceyumincludedir = $(pkgincludedir)/source/yum

sourceyuminclude_HEADERS =		\
	YUMSourceCacher.h


noinst_LTLIBRARIES =	lib@PACKAGE@2_source_yum.la

## ##################################################

lib@PACKAGE@2_source_yum_la_SOURCES =	\
	YUMSourceCacher.cc

## ##################################################
