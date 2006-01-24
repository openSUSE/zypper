#
# Makefile.am for zypp/solver/testsuite/src
#

SUBDIRS = helix

INCLUDES = -I/usr/include/libxml2 -Ihelix
LDADD =	$(top_srcdir)/zypp/lib@PACKAGE@.la helix/lib@PACKAGE@_helix.la

# We're not ready for this yet.
#check_PROGRAMS = deptestomatic.multi
noinst_PROGRAMS = utils.single				\
		deptestomatic.multi			\
		helixparse

utils_single_SOURCES =					\
	utils_single.cc

deptestomatic_multi_SOURCES =				\
	deptestomatic.cc

helixparse_SOURCES =					\
	helixparse.cc
