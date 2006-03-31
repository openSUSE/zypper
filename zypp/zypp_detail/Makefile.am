## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS =

INCLUDES = -DZYPP_BASE_LOGGER_LOGGROUP=\"zypp_detail\"

## ##################################################

baseincludedir = $(pkgincludedir)/zypp_detail

baseinclude_HEADERS =	\
	ZYppImpl.h	\
	ZYppReadOnlyHack.h

noinst_LTLIBRARIES =	lib@PACKAGE@_zypp_detail.la

## ##################################################

lib@PACKAGE@_zypp_detail_la_SOURCES =	\
	ZYppImpl.cc

# lib@PACKAGE@_zypp_detail_la_LIBADD =

## ##################################################
