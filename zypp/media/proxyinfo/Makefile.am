## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS =

INCLUDES = -I$(oldincludedir)/libxml2

## ##################################################

mediaproxyinfoincludedir = $(pkgincludedir)/media/proxyinfo

mediaproxyinfoinclude_HEADERS = 		\
	ProxyInfoImpl.h		\
	ProxyInfos.h		\
	ProxyInfoSysconfig.h


noinst_LTLIBRARIES =	lib@PACKAGE@_media_proxyinfo.la

## ##################################################

lib@PACKAGE@_media_proxyinfo_la_SOURCES = \
	ProxyInfoSysconfig.cc

lib@PACKAGE@_media_proxyinfo_la_LIBADD =

## ##################################################
