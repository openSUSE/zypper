INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(selectiontagparsertest_SRCS
SelectionTagParserTest.cc
)

KDE3_AUTOMOC(${selectiontagparsertest_SRCS})

KDE3_ADD_EXECUTABLE(selectiontagparsertest ${selectiontagparsertest_SRCS})

TARGET_LINK_LIBRARIES(selectiontagparsertest  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

SET(patterntagparsertest_SRCS
PatternTagParserTest.cc
)

KDE3_AUTOMOC(${patterntagparsertest_SRCS})

KDE3_ADD_EXECUTABLE(patterntagparsertest ${patterntagparsertest_SRCS})

TARGET_LINK_LIBRARIES(patterntagparsertest  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

SET(producttagparsertest_SRCS
ProductMetadataParserTest.cc
)

KDE3_AUTOMOC(${producttagparsertest_SRCS})

KDE3_ADD_EXECUTABLE(producttagparsertest ${producttagparsertest_SRCS})

TARGET_LINK_LIBRARIES(producttagparsertest  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

SET(mediatagparsertest_SRCS
MediaMetadataParserTest.cc
)

KDE3_AUTOMOC(${mediatagparsertest_SRCS})

KDE3_ADD_EXECUTABLE(mediatagparsertest ${mediatagparsertest_SRCS})

TARGET_LINK_LIBRARIES(mediatagparsertest  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

SET(suseparser_SRCS
suseparser.cc
KeyRingCallbacks.h
)

KDE3_AUTOMOC(${suseparser_SRCS})

KDE3_ADD_EXECUTABLE(suseparser ${suseparser_SRCS})

TARGET_LINK_LIBRARIES(suseparser  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

SET(source_read_test_SRCS
source-read-test.cc
)

KDE3_AUTOMOC(${source_read_test_SRCS})

KDE3_ADD_EXECUTABLE(source_read_test ${source_read_test_SRCS})

TARGET_LINK_LIBRARIES(source_read_test  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

SET(content_file_test_SRCS
content_file_test.cc
)

KDE3_AUTOMOC(${content_file_test_SRCS})

KDE3_ADD_EXECUTABLE(content_file_test ${content_file_test_SRCS})

TARGET_LINK_LIBRARIES(content_file_test  ${QT_AND_KDECORE_LIBS} boost_unit_test_framework )


########### install files ###############




#original Makefile.am contents follow:

##
## Makefile.am for testsuite/source/tests
##
#
#LDADD =	$(top_srcdir)/zypp/lib@PACKAGE@.la $(top_srcdir)/testsuite/src/utils/lib@PACKAGE@_testsuite_utils.la
#
#INCLUDES = 						\
#	-DZYPP_BASE_LOGGER_LOGGROUP=\"testsuite\"
#
#noinst_PROGRAMS = \
#	selectiontagparsertest	\
#	patterntagparsertest	\
#	producttagparsertest	\
#	mediatagparsertest	\
#	suseparser \
#	source_read_test \
#	content_file_test
#
## does not compile
## packagetagparsertest_SOURCES = PackageTagParserTest.cc
#	
#selectiontagparsertest_SOURCES = SelectionTagParserTest.cc
#patterntagparsertest_SOURCES = PatternTagParserTest.cc
#producttagparsertest_SOURCES = ProductMetadataParserTest.cc
#mediatagparsertest_SOURCES = MediaMetadataParserTest.cc
#suseparser_SOURCES = suseparser.cc KeyRingCallbacks.h
#source_read_test_SOURCES = source-read-test.cc
#content_file_test_SOURCES = content_file_test.cc
#content_file_test_LDADD = $(LDADD) -lboost_unit_test_framework