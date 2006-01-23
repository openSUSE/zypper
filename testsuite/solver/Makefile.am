#
# Makefile.am for zypp/solver/testsuite
#
SUBDIRS = src single.out
AUTOMAKE_OPTIONS = dejagnu
PACKAGE = solver

EXTRA_DIST = config/*.exp				\
	    lib/*.exp					\
	    $(PACKAGE).test/*.exp			\
	    data.deptestomatic/*-tests/*.xml		\
	    data.deptestomatic/*-tests/*.solution	\
	    data.deptestomatic/*-tests/README

dist-local: clean-local

clean-local:
	rm -rf data.deptestomatic/*/*.out
	rm -rf data.deptestomatic/*/*.diff
	rm -rf data.deptestomatic/*/*.mistake
	rm -rf single.out/out
