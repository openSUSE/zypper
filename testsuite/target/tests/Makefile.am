#
# Makefile.am for testsuite/target/tests
#

LDADD =	$(top_srcdir)/zypp/lib@PACKAGE@.la $(top_srcdir)/testsuite/src/utils/lib@PACKAGE@_testsuite_utils.la

noinst_PROGRAMS = hal librpmDb RpmDb storagetargettest import_old_sources_test dumpstore

hal_SOURCES = hal.cc

librpmDb_SOURCES = librpmDb.cc 
RpmDb_SOURCES = RpmDb.cc

storagetargettest_SOURCES = storagetargettest.cc Benchmark.cc Benchmark.h
dumpstore_SOURCES = dumpstore.cc

import_old_sources_test_SOURCES = import_old_sources_test.cc
