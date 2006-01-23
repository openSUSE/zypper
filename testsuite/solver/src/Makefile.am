#
# Makefile.am for zypp/solver/testsuite/src
#

#SUBDIRS = helix

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
