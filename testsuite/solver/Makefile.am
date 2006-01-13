#
# Makefile.am for zypp/solver/testsuite
#
SUBDIRS = single.out
AUTOMAKE_OPTIONS = dejagnu
PACKAGE = libzypp_solver

INCLUDES=						\
	-I$(top_srcdir)/src				\
	-I$(includedir)					\
	-I/usr/include/YaST2				\
	-I/usr/include/libxml2 -I/usr/include/rpm	\
        -DDBUS_API_SUBJECT_TO_CHANGE			\
	-I/usr/include/dbus-1.0				\
	-I/usr/lib64/dbus-1.0/include			\
	-I/opt/gnome/include/glib-2.0			\
	-I/opt/gnome/lib64/glib-2.0/include		\
	-I/usr/include/hal				\
	-DG_LOG_DOMAIN=\"testsuite\"			\
	-Wall

LIBZYPP_LIBS = -lxml2 -lz -lm -lpthread -lrt -lz -lbz2 -lzypp_base

# We're not ready for this yet.
#check_PROGRAMS = deptestomatic.multi
noinst_PROGRAMS = utils.single				\
		deptestomatic.multi

utils_single_SOURCES =					\
	utils_single.cc

utils_single_LDFLAGS =					\
	-L$(top_srcdir)/zypp/solver/temporary/.libs	\
	-L$(top_srcdir)/zypp/base/.libs

utils_single_LDADD =					\
	-lzypp_solver_temporary				\
	$(LIBZYPP_LIBS)


deptestomatic_multi_SOURCES =				\
	deptestomatic.cc

deptestomatic_multi_LDFLAGS =				\
	-L$(top_srcdir)/zypp/solver/detail/.libs	\
	-L$(top_srcdir)/zypp/solver/temporary/.libs	\
	-L$(top_srcdir)/zypp/base/.libs

deptestomatic_multi_LDADD =				\
	-lzypp_solver_detail				\
	-lzypp_solver_temporary				\
	-lzypp_solver_detail				\
	$(LIBZYPP_LIBS)

EXTRA_DIST = config/*.exp lib/*.exp $(PACKAGE).test/*.exp

clean-local:
	rm -rf data.deptestomatic/*/out
	rm -rf data.deptestomatic/*/out
	rm -rf single.out/out
