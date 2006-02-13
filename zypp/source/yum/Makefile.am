## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS =

INCLUDES = 						\
	-DZYPP_BASE_LOGGER_LOGGROUP=\"yum\"

## ##################################################

sourceyumincludedir = $(pkgincludedir)/source/yum

sourceyuminclude_HEADERS =		\
	YUMSourceImpl.h			\
	YUMScriptImpl.h			\
	YUMMessageImpl.h		\
	YUMPackageImpl.h		\
	YUMPatchImpl.h			\
	YUMGroupImpl.h			\
	YUMPatternImpl.h		\
	YUMProductImpl.h


noinst_LTLIBRARIES =	lib@PACKAGE@_source_yum.la

## ##################################################

lib@PACKAGE@_source_yum_la_SOURCES =	\
	YUMSourceImpl.cc		\
	YUMScriptImpl.cc		\
	YUMMessageImpl.cc		\
	YUMPackageImpl.cc		\
	YUMPatchImpl.cc			\
	YUMGroupImpl.cc			\
	YUMPatternImpl.cc		\
	YUMProductImpl.cc	

## ##################################################
