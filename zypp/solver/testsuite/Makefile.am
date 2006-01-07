#
# Makefile.am for src/testsuite
#
SUBDIRS = lib config libzypp_solver.test single.out
AUTOMAKE_OPTIONS = dejagnu
PACKAGE = libzypp_solver

INCLUDES=						\
	-I$(top_srcdir)/src				\
	-I$(includedir)					\
	-I/usr/include/YaST2				\
	-I/usr/include/libxml2 -I/usr/include/rpm	\
	-DG_LOG_DOMAIN=\"testsuite\"			\
	-Wall

LIBZYPP_LIBS = -lxml2 -lz -lm -ly2util -lpthread -lrt -lz -lbz2 -lzypp

# We're not ready for this yet.
#check_PROGRAMS = deptestomatic.multi
noinst_PROGRAMS = utils.single				\
		deptestomatic.multi

utils_single_SOURCES =					\
	utils_single.cc

utils_single_LDFLAGS =					\
	-L$(top_srcdir)/zypp/solver/temporary/.libs	\
	-L$(top_srcdir)/zypp

utils_single_LDADD =					\
	-lzypp_solver_temporary				\
	$(LIBZYPP_LIBS)


deptestomatic_multi_SOURCES =				\
	deptestomatic.cc

deptestomatic_multi_LDFLAGS =				\
	-L$(top_srcdir)/zypp/solver/detail/.libs	\
	-L$(top_srcdir)/zypp/solver/temporary/.libs	\
	-L$(top_srcdir)/zypp

deptestomatic_multi_LDADD =				\
	-lzypp_solver_detail				\
	-lzypp_solver_temporary				\
	-lzypp_solver_detail				\
	$(LIBZYPP_LIBS)



