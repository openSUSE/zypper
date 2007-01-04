INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(SourceCacheInitializer_SRCS
SourceCacheInitializer.cc
)

KDE3_AUTOMOC(${SourceCacheInitializer_SRCS})

IF(KDE3_BUILD_TESTS)

KDE3_ADD_EXECUTABLE(SourceCacheInitializer ${SourceCacheInitializer_SRCS})

TARGET_LINK_LIBRARIES(SourceCacheInitializer  ${QT_AND_KDECORE_LIBS} boost_unit_test_framework )

ENDIF(KDE3_BUILD_TESTS)

########### install files ###############




#original Makefile.am contents follow:

### Process this file with automake to produce Makefile.in
### ##################################################
#
## Do NOT list devel.* in subdirs. It's developers playgound and
## must not prevent anything from building.
#SUBDIRS =
#
### ##################################################
#
#TESTS = SourceCacheInitializer
#
#check_PROGRAMS = SourceCacheInitializer
#
### ##################################################
#
#LDADD =	$(top_srcdir)/zypp/lib@PACKAGE@.la $(top_srcdir)/zypp2/lib@PACKAGE@2.la
#
### ##################################################
#
#SourceCacheInitializer_SOURCES = SourceCacheInitializer.cc
#SourceCacheInitializer_LDADD = $(LDADD) -lboost_unit_test_framework
#
### ##################################################
#
#.PHONY:        always
#
#$(noinst_PROGRAMS):    $(top_srcdir)/zypp/lib@PACKAGE@.la
#
##$(top_srcdir)/zypp/lib@PACKAGE@.la:     always
##	$(MAKE) -C $(top_srcdir)/zypp
#
### ##################################################
