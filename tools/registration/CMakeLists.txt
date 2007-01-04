INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/tools/registration ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### install files ###############




#original Makefile.am contents follow:

### Process this file with automake to produce Makefile.in
### ##################################################
#
#libexecdir=$(exec_prefix)/lib/@PACKAGE@
#
#libexec_PROGRAMS = zypp-query-pool
#
##bindir = $(libexecdir)/zypp
##bin_PROGRAMS = zypp-query-pool
#
### ##################################################
#
#INCLUDES = -I$(top_srcdir)/tools/registration -I$(oldincludedir)/libxml2
#AM_LDFLAGS =
#
#AM_CXXFLAGS =
## gperf:	-pg
#
#LDADD =		-L$(top_srcdir)/zypp/.libs -lzypp
#
### ##################################################
#
#zypp_query_pool_SOURCES = 	zypp-query-pool.cc keyring-callbacks.h
#
##zmart_LDFLAGS =	-static
#
#.PHONY:        always
#
##$(noinst_PROGRAMS):    $(top_srcdir)/zypp/lib@PACKAGE@.la
#
#$(top_srcdir)/zypp/lib@PACKAGE@.la:     always
#	$(MAKE) -C $(top_srcdir)/zypp
#
### ##################################################
