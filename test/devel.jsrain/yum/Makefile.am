#
# Makefile.am for packagemanager/testsuite/devel.mir
#

SUBDIRS = YUMParser.test

AUTOMAKE_OPTIONS = dejagnu

PACKAGE = YUMParser

INCLUDES = -I$(top_srcdir)/zypp/parser/yum -I$(top_srcdir)/zypp -I$(oldincludedir)/libxml2 -I$(includedir)


check_PROGRAMS = YUMtest

YUMtest_SOURCES = YUMtest.cc
YUMtest_LDADD = $(top_builddir)/zypp/libzypp.la /usr/lib/libxml2.a /usr/lib/libz.a
	
YUMtest_LDFLAGS = -static

EXTRA_DIST = testit.sh
