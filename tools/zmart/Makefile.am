## Process this file with automake to produce Makefile.in
## ##################################################

noinst_PROGRAMS = zmart

## ##################################################

INCLUDES = -I$(oldincludedir)/libxml2
AM_LDFLAGS =

AM_CXXFLAGS =
# gperf:	-pg

LDADD =		-L$(top_srcdir)/zypp/.libs -lzypp -lboost_regex -lboost_program_options -lxml2 -lz

## ##################################################

zmart_SOURCES = 	zmart.cc
zmart_LDFLAGS =	-static

.PHONY:        always

#$(noinst_PROGRAMS):    $(top_srcdir)/zypp/lib@PACKAGE@.la

$(top_srcdir)/zypp/lib@PACKAGE@.la:     always
	$(MAKE) -C $(top_srcdir)/zypp

## ##################################################
