INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(@PACKAGE@_parser_yum2_STAT_SRCS
YUMPrimaryParser.cc
)

KDE3_AUTOMOC(${@PACKAGE@_parser_yum2_STAT_SRCS})

ADD_LIBRARY(@PACKAGE@_parser_yum2 STATIC ${@PACKAGE@_parser_yum2_STAT_SRCS})


########### install files ###############

INSTALL_FILES(  FILES  YUMPrimaryParser.h )



#original Makefile.am contents follow:

### Process this file with automake to produce Makefile.in
### ##################################################
#
#SUBDIRS =
#
#INCLUDES = -I$(oldincludedir)/libxml2 -I..	\
#	-DZYPP_BASE_LOGGER_LOGGROUP=\"parser-yum2\"
#
### ##################################################
#
#parseryum2includedir = $(pkgincludedir)/parser/yum2
#
#parseryum2include_HEADERS = \
#	YUMPrimaryParser.h
#
#noinst_LTLIBRARIES =	lib@PACKAGE@_parser_yum2.la
#
### ##################################################
#
#lib@PACKAGE@_parser_yum2_la_SOURCES = \
#	YUMPrimaryParser.cc
#
### ##################################################
