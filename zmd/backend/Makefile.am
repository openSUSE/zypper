#
# Makefile.am for zmd/backend
#
SUBDIRS = dbsource

PKG_MANAGEMENT_LIBS = dbsource/lib@PACKAGE@_dbsource.la $(top_srcdir)/zypp/lib@PACKAGE@.la
PKG_MANAGEMENT_CFLAGS = -I$(top_srcdir)

query_systemdir = $(libdir)/zmd
query_system_PROGRAMS = query-system
query_system_SOURCES = query-system.cc
query_system_CFLAGS = $(PKG_MANAGEMENT_CFLAGS) $(SQLITE_CFLAGS)
query_system_LDADD = $(PKG_MANAGEMENT_LIBS) $(SQLITE_LIBS)

query_filesdir = $(libdir)/zmd
query_files_PROGRAMS = query-files
query_files_SOURCES = query-files.cc
query_files_CFLAGS = $(PKG_MANAGEMENT_CFLAGS) $(SQLITE_CFLAGS)
query_files_LDADD = $(PKG_MANAGEMENT_LIBS) $(SQLITE_LIBS)


