#
# Makefile.am for testsuite/source
#
SUBDIRS = tests
AUTOMAKE_OPTIONS = dejagnu
PACKAGE = source

EXTRA_DIST = lib/*.exp config/*.exp $(PACKAGE).test/*.exp

clean-local:
	rm -rf *.log
	rm -rf *.exp
