## Process this file with automake to produce Makefile.in
## ##################################################

noinst_PROGRAMS = read_plaindir_source

## ##################################################

INCLUDES = -I$(oldincludedir)/libxml2
AM_LDFLAGS =

AM_CXXFLAGS =
# gperf:	-pg

LDADD =		-L$(top_srcdir)/zypp/.libs -lzypp -lboost_regex -lxml2 -lz $(top_srcdir)/testsuite/src/utils/lib@PACKAGE@_testsuite_utils.la

## ##################################################

read_plaindir_source_SOURCES = 	read_plaindir_source.cc
read_plaindir_source_LDFLAGS =	-static

.PHONY:        always

$(noinst_PROGRAMS):    $(top_srcdir)/zypp/lib@PACKAGE@.la

$(top_srcdir)/zypp/lib@PACKAGE@.la:     always
	$(MAKE) -C $(top_srcdir)/zypp

## ##################################################
