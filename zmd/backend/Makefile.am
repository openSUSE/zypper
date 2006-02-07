#
# Makefile.am for zmd/backend
#
SUBDIRS = dbsource

PKG_MANAGEMENT_LIBS = dbsource/lib@PACKAGE@_dbsource.la $(top_srcdir)/zypp/lib@PACKAGE@.la
PKG_MANAGEMENT_CFLAGS = -I$(top_srcdir)

CFLAGS = $(PKG_MANAGEMENT_CFLAGS) $(SQLITE_CFLAGS)
LDADD = $(PKG_MANAGEMENT_LIBS) $(SQLITE_LIBS)

bindir = $(libdir)/zmd

bin_PROGRAMS =	\
	query-system		\
	query-files		\
	resolve-dependencies

query_system_SOURCES = query-system.cc
query_files_SOURCES = query-files.cc
resolve_dependencies_SOURCES = resolve-dependencies.cc add-transactions.cc add-transactions.h
