#
# Makefile.am for zmd/backend
#
SUBDIRS = dbsource

PKG_MANAGEMENT_LIBS = dbsource/lib@PACKAGE@_dbsource.la $(top_srcdir)/zypp/lib@PACKAGE@.la
PKG_MANAGEMENT_CFLAGS = -I$(top_srcdir)

CFLAGS = $(PKG_MANAGEMENT_CFLAGS) $(SQLITE_CFLAGS)
LDADD = $(PKG_MANAGEMENT_LIBS) $(SQLITE_LIBS)

noinst_HEADERS = zmd-backend.h

bindir = $(libdir)/zmd

bin_PROGRAMS =	\
	query-system		\
	query-files		\
	resolve-dependencies	\
	transact

query_system_SOURCES = query-system.cc
query_system_INCLUDES = -DZYPP_BASE_LOGGER_LOGGROUP=\"query-system\"
query_files_SOURCES = query-files.cc
query_files_INCLUDES = -DZYPP_BASE_LOGGER_LOGGROUP=\"query-files\"
resolve_dependencies_SOURCES = resolve-dependencies.cc transactions.cc transactions.h
resolve_dependencies_INCLUDES = -DZYPP_BASE_LOGGER_LOGGROUP=\"resolve-dependencies\"
transact_SOURCES = transact.cc transactions.cc transactions.h
transact_INCLUDES = -DZYPP_BASE_LOGGER_LOGGROUP=\"transact\"

