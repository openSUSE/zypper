## Process this file with automake to produce Makefile.in
## ##################################################

noinst_PROGRAMS = test lock testbed aj

## ##################################################

INCLUDES =

AM_LDFLAGS =

AM_CXXFLAGS =
# gperf:	-pg

LDADD =		-L$(top_srcdir)/zypp/.libs -lzypp -lboost_regex

## ##################################################

test_SOURCES = 	zypp-keyring.cc
test_LDFLAGS =	-static

lock_SOURCES = 	zypp-lock.cc
lock_LDFLAGS =	-static

testbed_SOURCES = testbed.cc
testbed_LDFLAGS = -static

aj_SOURCES = 	aj.cc
aj_LDFLAGS =	-static

.PHONY:        always

$(noinst_PROGRAMS):    $(top_srcdir)/zypp/lib@PACKAGE@.la

$(top_srcdir)/zypp/lib@PACKAGE@.la:     always
	$(MAKE) -C $(top_srcdir)/zypp

## ##################################################
