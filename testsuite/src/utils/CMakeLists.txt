INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(@PACKAGE@_testsuite_utils_STAT_SRCS
TestUtils.cc
)

KDE3_AUTOMOC(${@PACKAGE@_testsuite_utils_STAT_SRCS})

ADD_LIBRARY(@PACKAGE@_testsuite_utils STATIC ${@PACKAGE@_testsuite_utils_STAT_SRCS})


########### install files ###############




#original Makefile.am contents follow:

### Process this file with automake to produce Makefile.in
### ##################################################
#
#SUBDIRS =
#
#INCLUDES = -DZYPP_BASE_LOGGER_LOGGROUP=\"testsuite_utils\"
#
### ##################################################
#
###targetincludedir = $(pkgincludedir)/target
#
#noinst_HEADERS =			\
#	TestUtils.h
#
### ##################################################
#
#noinst_LTLIBRARIES =	lib@PACKAGE@_testsuite_utils.la
#
### ##################################################
#
#lib@PACKAGE@_testsuite_utils_la_SOURCES =	\
#	TestUtils.cc
#	
#lib@PACKAGE@_testsuite_utils_la_LDFLAGS = @LIBZYPP_VERSION_INFO@
#
###lib@PACKAGE@_target_la_LIBADD = \
###	rpm/lib@PACKAGE@_target_rpm.la	\
###	hal/lib@PACKAGE@_target_hal.la \
###	modalias/lib@PACKAGE@_target_modalias.la \
###	store/lib@PACKAGE@_target_store.la
#
### ##################################################
