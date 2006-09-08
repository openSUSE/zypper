## Process this file with automake to produce Makefile.in
## ##################################################

noinst_PROGRAMS = zmart zypp-checkpatches

## ##################################################

INCLUDES = -I$(top_srcdir)/tools/zmart -I$(oldincludedir)/libxml2
AM_LDFLAGS =

AM_CXXFLAGS =
# gperf:	-pg

LDADD =		-L$(top_srcdir)/zypp/.libs -lzypp -lboost_regex -lboost_program_options -lxml2 -lz

## ##################################################

zmart_SOURCES = 	zmart.cc zmart-sources.cc zmart-misc.cc \
	AliveCursor.h \
	zmart-keyring-callbacks.h  \
	zmart-rpm-callbacks.h  \
	zmart-media-callbacks.h  \
	zmart-source-callbacks.h \
	zmart-sources.h \
	zmart-misc.h \
  zmart.h

zypp_checkpatches_SOURCES = 	zypp-checkpatches.cc zmart-sources.cc \
	checkpatches-keyring-callbacks.h
#zmart_LDFLAGS =	-static

.PHONY:        always

#$(noinst_PROGRAMS):    $(top_srcdir)/zypp/lib@PACKAGE@.la

$(top_srcdir)/zypp/lib@PACKAGE@.la:     always
	$(MAKE) -C $(top_srcdir)/zypp

## ##################################################
