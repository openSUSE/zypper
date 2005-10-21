## Process this file with automake to produce Makefile.in
## ##################################################

noinst_PROGRAMS =	Main Main.debug

## ##################################################

INCLUDES =

AM_LDFLAGS =

AM_CXXFLAGS =	-pg

LDADD =		$(top_srcdir)/zypp/lib@PACKAGE@.la

## ##################################################

Main_SOURCES = 		Main.cc

Main_debug_SOURCES = 	$(Main_SOURCES)
Main_debug_LDFLAGS =	-static

## ##################################################

.PHONY:        always

$(noinst_PROGRAMS):    $(top_srcdir)/zypp/lib@PACKAGE@.la

$(top_srcdir)/zypp/lib@PACKAGE@.la:     always
	$(MAKE) -C $(top_srcdir)/zypp

## ##################################################
