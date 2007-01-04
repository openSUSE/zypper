INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(test_SRCS
zypp-keyring.cc
)

KDE3_AUTOMOC(${test_SRCS})

KDE3_ADD_EXECUTABLE(test ${test_SRCS})

TARGET_LINK_LIBRARIES(test  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

SET(lock_SRCS
zypp-lock.cc
)

KDE3_AUTOMOC(${lock_SRCS})

KDE3_ADD_EXECUTABLE(lock ${lock_SRCS})

TARGET_LINK_LIBRARIES(lock  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

SET(testbed_SRCS
testbed.cc
)

KDE3_AUTOMOC(${testbed_SRCS})

KDE3_ADD_EXECUTABLE(testbed ${testbed_SRCS})

TARGET_LINK_LIBRARIES(testbed  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

SET(aj_SRCS
aj.cc
)

KDE3_AUTOMOC(${aj_SRCS})

KDE3_ADD_EXECUTABLE(aj ${aj_SRCS})

TARGET_LINK_LIBRARIES(aj  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

SET(yumparser_SRCS
yum-parser.cc
)

KDE3_AUTOMOC(${yumparser_SRCS})

KDE3_ADD_EXECUTABLE(yumparser ${yumparser_SRCS})

TARGET_LINK_LIBRARIES(yumparser  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

SET(media_SRCS
media-glob.cc
)

KDE3_AUTOMOC(${media_SRCS})

KDE3_ADD_EXECUTABLE(media ${media_SRCS})

TARGET_LINK_LIBRARIES(media  ${QT_AND_KDECORE_LIBS} )


########### install files ###############




#original Makefile.am contents follow:

### Process this file with automake to produce Makefile.in
### ##################################################
#
#noinst_PROGRAMS = test lock testbed aj yumparser media
#
### ##################################################
#
#INCLUDES = -I$(oldincludedir)/libxml2
#AM_LDFLAGS =
#
#AM_CXXFLAGS =
## gperf:	-pg
#
#LDADD =		-L$(top_srcdir)/zypp/.libs -L$(top_srcdir)/zypp2/.libs -lzypp -lzypp2 -lboost_regex -lxml2 -lz $(top_srcdir)/testsuite/src/utils/lib@PACKAGE@_testsuite_utils.la
#
### ##################################################
#
#media_SOURCES = media-glob.cc
#media_LDFLAGS =	-static
#
#test_SOURCES = 	zypp-keyring.cc
#test_LDFLAGS =	-static
#
#lock_SOURCES = 	zypp-lock.cc
#lock_LDFLAGS =	-static
#
#testbed_SOURCES = testbed.cc
#testbed_LDFLAGS = -static
#
#aj_SOURCES = 	aj.cc
#aj_LDFLAGS =	-static
#
#yumparser_SOURCES = 	yum-parser.cc
#yumparser_LDFLAGS =	-static
##yumparser_LDADD = 
#
#.PHONY:        always
#
#$(noinst_PROGRAMS):    $(top_srcdir)/zypp/lib@PACKAGE@.la $(top_srcdir)/zypp2/lib@PACKAGE@2.la
#
#$(top_srcdir)/zypp/lib@PACKAGE@.la:     always
#	$(MAKE) -C $(top_srcdir)/zypp
#
### ##################################################
