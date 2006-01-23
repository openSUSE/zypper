#
# Makefile.am for testsuite/parser
#
SUBDIRS = tests
AUTOMAKE_OPTIONS = dejagnu
PACKAGE = parser

LDADD = $(top_srcdir)/zypp/lib@PACKAGE@.la

noinst_PROGRAMS = YUMtest ProductMetadataParserTest

YUMtest_SOURCES = YUMtest.cc

ProductMetadataTest_SOURCES = ProductMetadataParserTest.cc

EXTRA_DIST = lib/*.exp config/*.exp $(PACKAGE).test/*.exp

clean-local:
	rm -rf *.log
	rm -rf *.exp
	rm -rf tmp*
