#
# Makefile.am for zypp/target/testsuite
#
SUBDIRS = tests
AUTOMAKE_OPTIONS = dejagnu
PACKAGE = target

EXTRA_DIST = lib/*.exp config/*.exp $(PACKAGE).test/*.exp

clean-local:
	rm -rf *.log
	rm -rf *.exp
