#
# Makefile.am for testsuite/source/tests
#

LDADD =	$(top_srcdir)/zypp/lib@PACKAGE@.la $(top_srcdir)/testsuite/utils/lib@PACKAGE@_testsuite_utils.la

INCLUDES = 						\
	-DZYPP_BASE_LOGGER_LOGGROUP=\"testsuite\"

noinst_PROGRAMS = \
	selectiontagparsertest	\
	patterntagparsertest	\
	producttagparsertest	\
	mediatagparsertest	\
	suseparser \
	source_read_test
	
selectiontagparsertest_SOURCES = SelectionTagParserTest.cc
packagetagparsertest_SOURCES = PackageTagParserTest.cc
patterntagparsertest_SOURCES = PatternTagParserTest.cc
producttagparsertest_SOURCES = ProductMetadataParserTest.cc
mediatagparsertest_SOURCES = MediaMetadataParserTest.cc
suseparser_SOURCES = suseparser.cc KeyRingCallbacks.h
source_read_test_SOURCES = source-read-test.cc
