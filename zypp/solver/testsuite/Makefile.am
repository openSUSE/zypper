#
# Makefile.am for src/testsuite
#
SUBDIRS = lib config libzypp_solver.test single.out
AUTOMAKE_OPTIONS = dejagnu
PACKAGE = libzypp_solver

INCLUDES=					\
	-I$(top_srcdir)/src			\
	-I$(includedir)				\
	-I/usr/include/YaST2			\
	-I/usr/include/libxml2 -I/usr/include/rpm	\
	-DG_LOG_DOMAIN=\"testsuite\"		\
	-Wall

LIBZYPP_LIBS = -lxml2 -lz -lm -ly2util -lpthread -lrt -lz -lbz2 -lzypp

# We're not ready for this yet.
#check_PROGRAMS = deptestomatic
noinst_PROGRAMS = edition.single	\
		spec.single	 	\
		utils.single		\
		deptestomatic.multi

edition_single_SOURCES =			\
	edition_single.cc

edition_single_LDFLAGS =			\
	-L$(top_srcdir)/zypp/solver/detail/.libs	\
	-L$(top_srcdir)/zypp

edition_single_LDADD =			\
	-lzypp_solver_detail			\
	$(LIBZYPP_LIBS)

spec_single_SOURCES =			\
	spec_single.cc

spec_single_LDFLAGS =			\
	-L$(top_srcdir)/zypp/solver/detail/.libs	\
	-L$(top_srcdir)/zypp

spec_single_LDADD =			\
	-lzypp_solver_detail			\
	$(LIBZYPP_LIBS)

utils_single_SOURCES =			\
	utils_single.cc

utils_single_LDFLAGS =			\
	-L$(top_srcdir)/zypp/solver/detail/.libs	\
	-L$(top_srcdir)/zypp

utils_single_LDADD =			\
	-lzypp_solver_detail			\
	$(LIBZYPP_LIBS)


deptestomatic_multi_SOURCES =		\
	deptestomatic_multi.cc

deptestomatic_multi_LDFLAGS =		\
	-L$(top_srcdir)/zypp/solver/detail/.libs	\
	-L$(top_srcdir)/zypp

deptestomatic_multi_LDADD =		\
	-lzypp_solver_detail			\
	$(LIBZYPP_LIBS)



