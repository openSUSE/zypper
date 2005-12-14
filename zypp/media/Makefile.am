## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS =

INCLUDES = -I$(oldincludedir)/libxml2

## ##################################################

include_HEADERS = 		\
	MediaException.h	\
	MediaAccess.h		\
	MediaHandler.h		\
	Mount.h			\
	MediaNFS.h		\
	MediaCD.h		\
	MediaDIR.h		\
	MediaDISK.h		\
	MediaSMB.h		\
	MediaCIFS.h		\
	ProxyInfo.h		\
	MediaCurl.h


noinst_LTLIBRARIES =	lib@PACKAGE@_media.la

## ##################################################

lib@PACKAGE@_media_la_SOURCES = \
	MediaException.cc	\
	MediaAccess.cc		\
	MediaHandler.cc		\
	Mount.cc		\
	MediaNFS.cc		\
	MediaCD.cc		\
	MediaDIR.cc		\
	MediaDISK.cc		\
	MediaSMB.cc		\
	MediaCIFS.cc		\
	ProxyInfo.cc		\
	MediaCurl.cc


lib@PACKAGE@_media_la_LIBADD =	-lcurl

## ##################################################
