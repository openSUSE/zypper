## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS = yum

## ##################################################

sourceincludedir = $(pkgincludedir)/source

sourceinclude_HEADERS =	\
	Source.h


noinst_LTLIBRARIES =	lib@PACKAGE@_source.la

## ##################################################

lib@PACKAGE@_source_la_SOURCES =	\
	Source.cc

lib@PACKAGE@_source_la_LIBADD =        yum/lib@PACKAGE@_source_yum.la

## ##################################################
