## Process this file with automake to produce Makefile.in
## ##################################################

noinst_PROGRAMS =	Main Parse Test Prof

## ##################################################

INCLUDES =

AM_LDFLAGS =

AM_CXXFLAGS = -pg

LDADD =		-L$(top_srcdir)/zypp/.libs -lzypp -lboost_regex

## ##################################################

Main_SOURCES = 	Main.cc
Main_LDFLAGS =	-static

## ##################################################

Parse_SOURCES = Parse.cc
Parse_LDFLAGS =	-static

## ##################################################

Test_SOURCES = Test.cc
Test_LDFLAGS =	-static

## ##################################################

PROF_OBJ = 	$(strip $(shell find $(top_srcdir)/zypp -name '*.o' | grep -v 'LogControl.o') ) \
		$(top_srcdir)/zypp/base/.libs/LogControl.o

Prof_SOURCES = Prof.cc
Prof_LDADD   = -lboost_regex -lboost_filesystem -lcurl -lidn -lblkid -luuid -lxml2 -lrpm -lutil
Prof_LDFLAGS = $(PROF_OBJ)

## ##################################################

.PHONY:        always

$(noinst_PROGRAMS):    $(top_srcdir)/zypp/lib@PACKAGE@.la

$(top_srcdir)/zypp/lib@PACKAGE@.la:     always
	$(MAKE) -C $(top_srcdir)/zypp

xtp:
	echo $(PROF_OBJ)

## ##################################################
