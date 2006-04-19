#
# Makefile.am for testsuite/target/tests
#

LDADD =	$(top_srcdir)/zypp/lib@PACKAGE@.la

noinst_PROGRAMS = hal librpmDb RpmDb storagetargettest import_old_sources_test

hal_SOURCES = hal.cc

librpmDb_SOURCES = librpmDb.cc 
RpmDb_SOURCES = RpmDb.cc

storagetargettest_SOURCES = storagetargettest.cc Benchmark.cc Benchmark.h

import_old_sources_test_SOURCES = import_old_sources_test.cc