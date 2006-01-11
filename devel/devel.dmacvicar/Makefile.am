## Process this file with automake to produce Makefile.in
## ##################################################

noinst_PROGRAMS =	test2 test2_debug

#Main Main.debug Parse

## ##################################################

INCLUDES =

AM_LDFLAGS = -pg

AM_CXXFLAGS =
# gperf:	-pg


# -ldbxml -lpathan -lxerces-c -lxquery
LDADD =		$(top_srcdir)/zypp/lib@PACKAGE@.la -lboost_regex -lboost_filesystem -ldb -ldb_cxx 
#LDADD =	-lboost_regex -lsqlite3 -dbxml

## ##################################################

test2_SOURCES = test2.cc

test2_debug_SOURCES = 	$(test2_SOURCES)
test2_debug_LDFLAGS =	-static

## ##################################################

test1_SOURCES = test-sqlite.cc

test1_debug_SOURCES = 	$(test1_SOURCES)
test1_debug_LDFLAGS =	-static

## ##################################################

#Main_SOURCES = 		Main.cc

#Main_debug_SOURCES = 	$(Main_SOURCES)
#Main_debug_LDFLAGS =	-static

## ##################################################

#Parse_SOURCES = 	Parse.cc
#Parse_LDFLAGS =		-static

## ##################################################

.PHONY:        always

$(noinst_PROGRAMS):    $(top_srcdir)/zypp/lib@PACKAGE@.la

$(top_srcdir)/zypp/lib@PACKAGE@.la:     always
	$(MAKE) -C $(top_srcdir)/zypp

## ##################################################
