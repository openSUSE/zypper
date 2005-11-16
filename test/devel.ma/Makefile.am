## Process this file with automake to produce Makefile.in
## ##################################################

noinst_PROGRAMS =	Main Main.debug PtrTest

## ##################################################

INCLUDES =

AM_LDFLAGS =

AM_CXXFLAGS =	-pg

LDADD =		$(top_srcdir)/zypp/lib@PACKAGE@.la

## ##################################################

Main_SOURCES = 		Main.cc main.cc

Main_debug_SOURCES = 	$(Main_SOURCES)
Main_debug_LDFLAGS =	-static

## ##################################################

PtrTest_SOURCES = 	PtrTest.cc

## ##################################################

.PHONY:        always

$(noinst_PROGRAMS):    $(top_srcdir)/zypp/lib@PACKAGE@.la

$(top_srcdir)/zypp/lib@PACKAGE@.la:     always
	$(MAKE) -C $(top_srcdir)/zypp

## ##################################################
