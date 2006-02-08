#
# Makefile.am for testsuite/source/tests
#

LDADD =	$(top_srcdir)/zypp/lib@PACKAGE@.la

noinst_PROGRAMS = \
	selectiontagparsertest	\
	patterntagparsertest	\
	producttagparsertest	\
	mediatagparsertest	\
	suseparser
	
selectiontagparsertest_SOURCES = SelectionTagParserTest.cc
patterntagparsertest_SOURCES = PatternTagParserTest.cc
producttagparsertest_SOURCES = ProductMetadataParserTest.cc
mediatagparsertest_SOURCES = MediaMetadataParserTest.cc
suseparser_SOURCES = suseparser.cc
