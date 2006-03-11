## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS =

## ##################################################

targetmodaliasincludedir = $(pkgincludedir)/target/modalias

targetmodaliasinclude_HEADERS =	\
	Modalias.h


noinst_LTLIBRARIES =	lib@PACKAGE@_target_modalias.la

## ##################################################

lib@PACKAGE@_target_modalias_la_SOURCES =	\
	Modalias.cc

lib@PACKAGE@_target_modalias_la_LIBADD =	\
	@MODALIAS_LIBS@

## ##################################################
