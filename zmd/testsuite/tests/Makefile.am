## Process this file with automake to produce Makefile.in
## ##################################################

noinst_PROGRAMS =	\
	catalogs	\
	sources

## ##################################################

LDADD = $(top_srcdir)/zmd/backend/dbsource/lib@PACKAGE@_dbsource.la \
	$(top_srcdir)/zypp/lib@PACKAGE@.la			    \
	$(SQLITE_LIBS)
	
CFLAGS = $(SQLITE_CFLAGS)

## ##################################################

catalogs_SOURCES = catalogs.cc

sources_SOURCES = sources.cc


