INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(read_plaindir_source_SRCS
read_plaindir_source.cc
)

KDE3_AUTOMOC(${read_plaindir_source_SRCS})

KDE3_ADD_EXECUTABLE(read_plaindir_source ${read_plaindir_source_SRCS})

TARGET_LINK_LIBRARIES(read_plaindir_source  ${QT_AND_KDECORE_LIBS} )


########### install files ###############




#original Makefile.am contents follow:

### Process this file with automake to produce Makefile.in
### ##################################################
#
#noinst_PROGRAMS = read_plaindir_source
#
### ##################################################
#
#INCLUDES = -I$(oldincludedir)/libxml2
#AM_LDFLAGS =
#
#AM_CXXFLAGS =
## gperf:	-pg
#
#LDADD =		-L$(top_srcdir)/zypp/.libs -lzypp -lboost_regex -lxml2 -lz
#
### ##################################################
#
#read_plaindir_source_SOURCES = 	read_plaindir_source.cc
#read_plaindir_source_LDFLAGS =	-static
#
#.PHONY:        always
#
#$(noinst_PROGRAMS):    $(top_srcdir)/zypp/lib@PACKAGE@.la
#
#$(top_srcdir)/zypp/lib@PACKAGE@.la:     always
#	$(MAKE) -C $(top_srcdir)/zypp
#
### ##################################################
