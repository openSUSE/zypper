## Process this file with automake to produce Makefile.in
## ##################################################

noinst_PROGRAMS = AOUT Prof

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

AOUT_SOURCES = AOUT.cc $(ADD_SOURCES)
AOUT_LDFLAGS = -static

## ##################################################

%: %.cc $(top_srcdir)/zypp/lib@PACKAGE@.la
	ln -sf $@.Po .deps/AOUT.Po
	ln -sf $@.cc AOUT.cc
	ln -sf $@.o  AOUT.o
	ls -l
	$(MAKE) AOUT
	mv AOUT $@

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
