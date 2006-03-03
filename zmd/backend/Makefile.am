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
	package-files		\
	parse-metadata		\
	find-file		\
	transact

query_system_SOURCES = query-system.cc
query_files_SOURCES = query-files.cc
resolve_dependencies_SOURCES = resolve-dependencies.cc transactions.cc transactions.h
transact_SOURCES = transact.cc transactions.cc transactions.h RpmCallbacks.h
parse_metadata_SOURCES = parse-metadata.cc
package_files_SOURCES = package-files.cc
find_file_SOURCES = find-file.cc
