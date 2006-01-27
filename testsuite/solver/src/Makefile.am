#
# Makefile.am for zypp/solver/testsuite/src
#

SUBDIRS = helix

INCLUDES = -I/usr/include/libxml2 -Ihelix
LDADD =	$(top_srcdir)/zypp/lib@PACKAGE@.la helix/lib@PACKAGE@_helix.la

# We're not ready for this yet.
#check_PROGRAMS = deptestomatic.multi
noinst_PROGRAMS = zypptestomatic.multi			\
		helixparse

zypptestomatic_multi_SOURCES =				\
	zypptestomatic.cc

helixparse_SOURCES =					\
	helixparse.cc
