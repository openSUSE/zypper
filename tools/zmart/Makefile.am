## Process this file with automake to produce Makefile.in
## ##################################################

bin_PROGRAMS = zypper
sbin_PROGRAMS = zypp-checkpatches zypp-checkpatches-wrapper
noinst_PROGRAMS = zmart

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
  zmart-updates.cc \
	zmart-updates.h \
	zmart-misc.h \
	zypper-tabulator.h \
	zypper-tabulator.cc \
  zmart.h

zypper_SOURCES = 	zypper.cc zmart-sources.cc zmart-misc.cc \
	AliveCursor.h \
	zmart-keyring-callbacks.h  \
	zmart-rpm-callbacks.h  \
	zmart-media-callbacks.h  \
	zmart-source-callbacks.h \
	zmart-sources.h \
	zmart-misc.h \
	zypper-tabulator.h \
	zypper-tabulator.cc \
  zmart.h

zypp_checkpatches_SOURCES = 	zypp-checkpatches.cc zmart-sources.cc \
	zypper-tabulator.h \
	zypper-tabulator.cc \
	zmart-updates.cc \
	zmart-updates.h \
	checkpatches-keyring-callbacks.h

zypp_checkpatches_wrapper_SOURCES = zypp-checkpatches-wrapper.c
# empty:
zypp_checkpatches_wrapper_LDADD =

#zmart_LDFLAGS =	-static

.PHONY:        always

#$(noinst_PROGRAMS):    $(top_srcdir)/zypp/lib@PACKAGE@.la

$(top_srcdir)/zypp/lib@PACKAGE@.la:     always
	$(MAKE) -C $(top_srcdir)/zypp

## ##################################################
