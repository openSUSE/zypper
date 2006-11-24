## Process this file with automake to produce Makefile.in
## ##################################################

noinst_PROGRAMS =	Draft Main Parse Test Prof Iorder Basic Update

## ##################################################

INCLUDES =

AM_LDFLAGS =

AM_CXXFLAGS = -pg

LDADD =		-L$(top_srcdir)/zypp/.libs -lzypp -lboost_regex

## ##################################################

ADD_SOURCES = 	xml/Node.cc		xml/Node.h	\
		xml/Reader.cc		xml/Reader.h	\
		xml/XmlString.cc	xml/XmlString.h	\
		xml/libxmlfwd.cc	xml/libxmlfwd.h

## ##################################################

Basic_SOURCES = Basic.cc
Basic_LDFLAGS =	-static

## ##################################################

Update_SOURCES = Update.cc
Update_LDFLAGS = -static

## ##################################################

Main_SOURCES = 	Main.cc $(ADD_SOURCES)
Main_LDFLAGS =	-static

## ##################################################

Parse_SOURCES = Parse.cc
Parse_LDFLAGS =	-static

## ##################################################

Test_SOURCES = 	Test.cc $(ADD_SOURCES)

Test_LDFLAGS =	-static

## ##################################################

Iorder_SOURCES = Iorder.cc
Iorder_LDFLAGS = -static

## ##################################################

Draft_SOURCES = Draft.cc
Draft_LDFLAGS =	-static

## ##################################################

PROF_OBJ = 	$(strip $(shell find $(top_srcdir)/zypp -name '*.o' | grep -v 'LogControl.o') ) \
		$(top_srcdir)/zypp/base/.libs/LogControl.o

Prof_SOURCES = Prof.cc
Prof_LDADD   = -lboost_regex -lboost_filesystem -lcurl -lidn -lblkid -luuid -lxml2 -lrpm -lutil
Prof_LDFLAGS = $(PROF_OBJ)

## ##################################################

.PHONY:		always

Tools.h:	$(top_srcdir)/zypp/lib@PACKAGE@.la

$(top_srcdir)/zypp/lib@PACKAGE@.la:     always
	$(MAKE) -C $(top_srcdir)/zypp

xtp:
	echo $(PROF_OBJ)

## ##################################################
