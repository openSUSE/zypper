#
# Makefile.am for zypp/target/testsuite/tests
#

INCLUDES=						\
	-I$(top_srcdir)/src				\
	-I$(includedir)					\
	-I/usr/include/libxml2 -I/usr/include/rpm	\
        -DDBUS_API_SUBJECT_TO_CHANGE			\
	@HAL_CFLAGS@					\
	-DG_LOG_DOMAIN=\"testsuite\"			\
	-Wall

LIBZYPP_LIBS = -lxml2 -lz -lm -ly2util -lpthread -lrt -lz -lbz2 -lzypp

noinst_PROGRAMS = hal

hal_SOURCES =					\
	hal.cc

hal_LDFLAGS =					\
	-L$(top_srcdir)/zypp/target/hal/.libs	\
	-L$(top_srcdir)/zypp

hal_LDADD =					\
	-lzypp_target_hal			\
	@HAL_LIBS@				\
	$(LIBZYPP_LIBS)
