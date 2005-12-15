## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS =

INCLUDES = -I$(oldincludedir)/libxml2

## ##################################################

include_HEADERS = 		\
	ProxyInfoImpl.h		\
	ProxyInfoSysconfig.h


noinst_LTLIBRARIES =	lib@PACKAGE@_media.la

## ##################################################

lib@PACKAGE@_media_la_SOURCES = \
	ProxyInfoSysconfig.cc


lib@PACKAGE@_media_la_LIBADD =	-lcurl

## ##################################################
