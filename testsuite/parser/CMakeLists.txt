ADD_SUBDIRECTORY( tests ) 

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(YUMtest_SRCS
YUMtest.cc
)

KDE3_AUTOMOC(${YUMtest_SRCS})

KDE3_ADD_EXECUTABLE(YUMtest ${YUMtest_SRCS})

TARGET_LINK_LIBRARIES(YUMtest  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

SET(YUMParserTest_SRCS
YUMParserTest.cc
)

KDE3_AUTOMOC(${YUMParserTest_SRCS})

KDE3_ADD_EXECUTABLE(YUMParserTest ${YUMParserTest_SRCS})

TARGET_LINK_LIBRARIES(YUMParserTest  ${QT_AND_KDECORE_LIBS} boost_unit_test_framework )


########### install files ###############




#original Makefile.am contents follow:

##
## Makefile.am for testsuite/parser
##
#SUBDIRS = tests
#AUTOMAKE_OPTIONS = dejagnu
#PACKAGE = parser
#
#LDADD = $(top_srcdir)/zypp/lib@PACKAGE@.la
#
#noinst_PROGRAMS = YUMtest YUMParserTest
#
#YUMtest_SOURCES = YUMtest.cc
#
#YUMParserTest_SOURCES = YUMParserTest.cc
#YUMParserTest_LDADD = $(LDADD) -lboost_unit_test_framework
#
##YUMLeakTest_SOURCES = YUMLeakTest.cc
#
#EXTRA_DIST = lib/*.exp config/*.exp $(PACKAGE).test/*.exp
#
#clean-local:
#	rm -rf *.log
#	rm -rf *.exp
#	rm -rf tmp*
#
#.PHONY:        always
#
#$(noinst_PROGRAMS):    $(top_srcdir)/zypp/lib@PACKAGE@.la