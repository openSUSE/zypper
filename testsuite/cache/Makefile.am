## Process this file with automake to produce Makefile.in
## ##################################################

# Do NOT list devel.* in subdirs. It's developers playgound and
# must not prevent anything from building.
SUBDIRS =

## ##################################################

TESTS = CacheInitializerTest

check_PROGRAMS = CacheInitializerTest

## ##################################################

LDADD =	$(top_srcdir)/zypp/lib@PACKAGE@.la $(top_srcdir)/zypp2/lib@PACKAGE@2.la

## ##################################################

CacheInitializerTest_SOURCES = CacheInitializer.cc
CacheInitializerTest_LDADD = $(LDADD) -lboost_unit_test_framework

## ##################################################

.PHONY:        always

$(noinst_PROGRAMS):    $(top_srcdir)/zypp/lib@PACKAGE@.la

#$(top_srcdir)/zypp/lib@PACKAGE@.la:     always
#	$(MAKE) -C $(top_srcdir)/zypp

## ##################################################
