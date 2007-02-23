## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS = proxyinfo

INCLUDES = -I$(oldincludedir)/libxml2	\
	$(CURL_CFLAGS)			\
	-DVERSION=\"@VERSION@\"		\
	-DZYPP_BASE_LOGGER_LOGGROUP=\"media\"


## ##################################################

mediaincludedir = $(pkgincludedir)/media

mediainclude_HEADERS = 		\
	MediaException.h	\
	MediaAccess.h		\
	MediaHandler.h		\
	MediaSource.h		\
	Mount.h			\
	MediaNFS.h		\
	MediaCD.h		\
	MediaDIR.h		\
	MediaDISK.h		\
	MediaSMB.h		\
	MediaCIFS.h		\
	ProxyInfo.h		\
	MediaCurl.h		\
	MediaISO.h		\
	MediaManager.h		\		
	MediaUserAuth.h


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
	MediaCurl.cc		\
	MediaISO.cc		\
	MediaManager.cc		\		
	MediaUserAuth.cc


lib@PACKAGE@_media_la_LIBADD =	proxyinfo/lib@PACKAGE@_media_proxyinfo.la \
				$(CURL_LIBS)

## ##################################################
