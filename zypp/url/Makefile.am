## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS =

## ##################################################

urlincludedir = $(pkgincludedir)/url

urlinclude_HEADERS =	\
	UrlBase.h	\
	UrlUtils.h

noinst_LTLIBRARIES =	lib@PACKAGE@_url.la

## ##################################################

lib@PACKAGE@_url_la_SOURCES =	\
	UrlUtils.cc	\
	UrlBase.cc

lib@PACKAGE@_url_la_LIBADD = -lboost_regex

## ##################################################
