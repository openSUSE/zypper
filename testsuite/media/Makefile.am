#
# Makefile.am for zypp/media/testsuite
#
SUBDIRS = tests
AUTOMAKE_OPTIONS = dejagnu
PACKAGE = media

EXTRA_DIST = lib/*.exp config/*.exp $(PACKAGE).test/*.exp

clean-local:
	rm -rf *.log
	rm -rf *.exp
