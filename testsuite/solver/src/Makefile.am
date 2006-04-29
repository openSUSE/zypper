#
# Makefile.am for zypp/solver/testsuite/src
#

SUBDIRS = helix

INCLUDES = -I/usr/include/libxml2	\
	-Ihelix				\
	-DZYPP_BASE_LOGGER_LOGGROUP=\"testsuite\"

LDADD =	$(top_srcdir)/zypp/lib@PACKAGE@.la helix/lib@PACKAGE@_helix.la

# We're not ready for this yet.
#check_PROGRAMS = deptestomatic.multi
noinst_PROGRAMS = deptestomatic.multi			\
		helixparse

deptestomatic_multi_SOURCES =				\
	deptestomatic.cc KeyRingCallbacks.h

helixparse_SOURCES =					\
	helixparse.cc
