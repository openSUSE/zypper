INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(installation_sources_SRCS
installation_sources.cc
)

KDE3_AUTOMOC(${installation_sources_SRCS})

KDE3_ADD_EXECUTABLE(installation_sources ${installation_sources_SRCS})

TARGET_LINK_LIBRARIES(installation_sources  ${QT_AND_KDECORE_LIBS} zypp )


########### install files ###############




#original Makefile.am contents follow:

### Process this file with automake to produce Makefile.in
### ##################################################
#
#noinst_PROGRAMS =	installation_sources
#
### ##################################################
#
#INCLUDES =
#
#AM_LDFLAGS =
#
#AM_CXXFLAGS =
#
#
### ##################################################
#
#installation_sources_SOURCES = 	installation_sources.cc
#installation_sources_LDADD = ../../zypp/libzypp.la
#
### ##################################################
#
##$(noinst_PROGRAMS):    $(top_srcdir)/zypp/lib@PACKAGE@.la
#
##$(top_srcdir)/zypp/lib@PACKAGE@.la:
##	$(MAKE) -C $(top_srcdir)/zypp
#
### ##################################################
