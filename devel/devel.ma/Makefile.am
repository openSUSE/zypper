## Process this file with automake to produce Makefile.in
## ##################################################

noinst_PROGRAMS =	Main Main.debug Parse

## ##################################################

INCLUDES =

AM_LDFLAGS =

AM_CXXFLAGS =
# gperf:	-pg

LDADD =		$(top_srcdir)/zypp/lib@PACKAGE@.la -lboost_regex

## ##################################################

Main_SOURCES = 		Main.cc Packages.cc

Main_debug_SOURCES = 	$(Main_SOURCES)
Main_debug_LDFLAGS =	-static

## ##################################################

Parse_SOURCES = 	Parse.cc
Parse_LDFLAGS =		-static

## ##################################################

.PHONY:        always

$(noinst_PROGRAMS):    $(top_srcdir)/zypp/lib@PACKAGE@.la

$(top_srcdir)/zypp/lib@PACKAGE@.la:     always
	$(MAKE) -C $(top_srcdir)/zypp

## ##################################################
