#
# Makefile.am for testsuite/parser
#
SUBDIRS = tests
AUTOMAKE_OPTIONS = dejagnu
PACKAGE = parser

LDADD = $(top_srcdir)/zypp/lib@PACKAGE@.la

noinst_PROGRAMS = YUMtest YUMLeakTest

YUMtest_SOURCES = YUMtest.cc
YUMLeakTest_SOURCES = YUMLeakTest.cc

EXTRA_DIST = lib/*.exp config/*.exp $(PACKAGE).test/*.exp

clean-local:
	rm -rf *.log
	rm -rf *.exp
	rm -rf tmp*
