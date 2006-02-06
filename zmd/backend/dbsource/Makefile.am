#
# Makefile.am for zmd/backend/dbsource
#

INCLUDES = 					\
	-I$(top_srcdir) -Wall			\
	-DG_LOG_DOMAIN=\"dbsource\"		\
	-DZYPP_BASE_LOGGER_LOGGROUP=\"dbsource\"

LDADD =	$(noinst_LTLIBRARIES) $(top_srcdir)/zypp/lib@PACKAGE@.la

noinst_HEADERS =				\
	DbSources.h				\
	DbSourceImpl.h				\
	DbPackageImpl.h				\
	DbAccess.h
	
noinst_LTLIBRARIES =	lib@PACKAGE@_dbsource.la

lib@PACKAGE@_dbsource_la_SOURCES =		\
	DbSourceImpl.cc				\
	DbPackageImpl.cc			\
	DbSources.cc				\
	DbAccess.cc 
