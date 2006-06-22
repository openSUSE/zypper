#
# Makefile.am for zmd/backend/dbsource
#

INCLUDES = 					\
	-I$(top_srcdir) $(ZYPP_CFLAGS) -Wall	\
	-DG_LOG_DOMAIN=\"sqlite-source\"		\
	-DZYPP_BASE_LOGGER_LOGGROUP=\"sqlite-source\"

sourceincludedir = $(pkgincludedir)/source

pkginclude_HEADERS =				\
	SqliteSources.h				\
	SqliteAccess.h

sourceinclude_HEADERS =				\
	SqliteSourceImpl.h				\
	SqlitePackageImpl.h				\
	SqliteAtomImpl.h				\
	SqliteScriptImpl.h				\
	SqliteMessageImpl.h				\
	SqliteLanguageImpl.h			\
	SqlitePatchImpl.h				\
	SqlitePatternImpl.h				\
	SqliteProductImpl.h

noinst_LTLIBRARIES =	lib@PACKAGE@_source_sqlite.la

lib@PACKAGE@_source_sqlite_la_SOURCES =			\
	SqliteSourceImpl.cc				\
	SqlitePackageImpl.cc			\
	SqliteAtomImpl.cc				\
	SqliteScriptImpl.cc				\
	SqliteMessageImpl.cc			\
	SqliteLanguageImpl.cc			\
	SqlitePatchImpl.cc				\
	SqlitePatternImpl.cc			\
	SqliteProductImpl.cc			\
	SqliteSources.cc				\
	SqliteAccess.cc 

lib@PACKAGE@_source_sqlite_la_LBADD = -lsqlite3
