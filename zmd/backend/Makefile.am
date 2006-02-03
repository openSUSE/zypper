#
# Makefile.am for zmd/backend
#

PKG_MANAGEMENT_LIBS = $(top_srcdir)/zypp/lib@PACKAGE@.la
PKG_MANAGEMENT_CFLAGS = -I$(top_srcdir)

RESOLVABLE_WRITER = \
	resolvable-writer.cc \
	resolvable-writer.h

query_systemdir = $(libdir)/zmd
query_system_PROGRAMS = query-system
query_system_SOURCES = query-system.cc $(RESOLVABLE_WRITER)
query_system_CFLAGS = $(PKG_MANAGEMENT_CFLAGS) $(SQLITE_CFLAGS)
query_system_LDADD = $(PKG_MANAGEMENT_LIBS) $(SQLITE_LIBS)

