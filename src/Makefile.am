## Process this file with automake to produce Makefile.in
## ##################################################

bin_PROGRAMS = zypper
sbin_PROGRAMS = zypp-checkpatches zypp-checkpatches-wrapper
noinst_PROGRAMS = zmart

## ##################################################

AM_CPPFLAGS = $(ZYPP_CFLAGS)
AM_LDFLAGS =

AM_CXXFLAGS =
# gperf:	-pg

## ##################################################

zmart_SOURCES = 	zmart.cc zmart-sources.cc zmart-misc.cc \
	AliveCursor.h \
	zmart-keyring-callbacks.h  \
	zmart-rpm-callbacks.h  \
	zmart-media-callbacks.h  \
	zmart-source-callbacks.h \
	zypper-callbacks.h \
	zypper-callbacks.cc \
	zmart-sources.h \
  zmart-updates.cc \
	zmart-updates.h \
	zmart-utils.h \
	zmart-utils.cc \
	zmart-misc.h \
	zypper-tabulator.h \
	zypper-tabulator.cc \
	zmart.h
zmart_LDADD =	$(ZYPP_LIBS) -lboost_regex -lboost_program_options


zypper_SOURCES = 	zypper.cc zmart-sources.cc zmart-misc.cc \
	AliveCursor.h \
	zmart-keyring-callbacks.h  \
	zmart-rpm-callbacks.h  \
	zmart-media-callbacks.h  \
	zmart-source-callbacks.h \
	zypper-callbacks.h \
	zypper-callbacks.cc \
	zmart-sources.h \
	zmart-misc.h \
	zypper-tabulator.h \
	zypper-tabulator.cc \
	zypper-search.h \
	zypper-search.cc \
	zypper-info.h \
	zypper-info.cc \
	zypper-getopt.h \
	zypper-getopt.cc \
	zmart.h
zypper_LDADD = $(ZYPP_LIBS) -lboost_regex

zypp_checkpatches_SOURCES = 	zypp-checkpatches.cc zmart-sources.cc \
	zypper-tabulator.h \
	zypper-tabulator.cc \
	zmart-updates.cc \
	zmart-updates.h \
	zmart-utils.cc \
	zmart-utils.h \
	checkpatches-keyring-callbacks.h
zypp_checkpatches_LDADD = $(ZYPP_LIBS) -lboost_regex -lboost_program_options

zypp_checkpatches_wrapper_SOURCES = zypp-checkpatches-wrapper.c
# empty:
zypp_checkpatches_wrapper_LDADD =

#zmart_LDFLAGS =	-static

## ##################################################
