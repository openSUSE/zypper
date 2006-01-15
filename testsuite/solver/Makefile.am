#
# Makefile.am for zypp/solver/testsuite
#
SUBDIRS = single.out
AUTOMAKE_OPTIONS = dejagnu
PACKAGE = solver

INCLUDES = -I/usr/include/libxml2
LDADD =	$(top_srcdir)/zypp/lib@PACKAGE@.la

# We're not ready for this yet.
#check_PROGRAMS = deptestomatic.multi
noinst_PROGRAMS = utils.single				\
		deptestomatic.multi

utils_single_SOURCES =					\
	utils_single.cc

deptestomatic_multi_SOURCES =				\
	deptestomatic.cc

EXTRA_DIST = config/*.exp lib/*.exp $(PACKAGE).test/*.exp data.deptestomatic

dist-local: clean-local

clean-local:
	rm -rf data.deptestomatic/*/*.out
	rm -rf data.deptestomatic/*/*.diff
	rm -rf data.deptestomatic/*/*.mistake
	rm -rf single.out/out
