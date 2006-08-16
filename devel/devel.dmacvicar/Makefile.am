## Process this file with automake to produce Makefile.in
## ##################################################

noinst_PROGRAMS = test lock testbed aj yumparser media

## ##################################################

INCLUDES = -I$(oldincludedir)/libxml2
AM_LDFLAGS =

AM_CXXFLAGS =
# gperf:	-pg

LDADD =		-L$(top_srcdir)/zypp/.libs -L$(top_srcdir)/zypp2/.libs -lzypp -lzypp2 -lboost_regex -lxml2 -lz $(top_srcdir)/testsuite/src/utils/lib@PACKAGE@_testsuite_utils.la

## ##################################################

media_SOURCES = media-glob.cc
media_LDFLAGS =	-static

test_SOURCES = 	zypp-keyring.cc
test_LDFLAGS =	-static

lock_SOURCES = 	zypp-lock.cc
lock_LDFLAGS =	-static

testbed_SOURCES = testbed.cc
testbed_LDFLAGS = -static

aj_SOURCES = 	aj.cc
aj_LDFLAGS =	-static

yumparser_SOURCES = 	yum-parser.cc
yumparser_LDFLAGS =	-static
#yumparser_LDADD = 

.PHONY:        always

$(noinst_PROGRAMS):    $(top_srcdir)/zypp/lib@PACKAGE@.la $(top_srcdir)/zypp2/lib@PACKAGE@2.la

$(top_srcdir)/zypp/lib@PACKAGE@.la:     always
	$(MAKE) -C $(top_srcdir)/zypp

## ##################################################
