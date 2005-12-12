## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS =

INCLUDES = -I$(oldincludedir)/libxml2

## ##################################################

include_HEADERS = 		\
	MediaException.h	\
	MediaAccess.h		\
	MediaHandler.h		\
	Mount.h

#	MediaCurl.h


noinst_LTLIBRARIES =	lib@PACKAGE@_media.la

## ##################################################

lib@PACKAGE@_media_la_SOURCES = \
	MediaAccess.cc		\
	MediaHandler.cc		\
	Mount.cc

#	MediaCurl.cc

lib@PACKAGE@_media_la_LIBADD =	-lcurl

## ##################################################
