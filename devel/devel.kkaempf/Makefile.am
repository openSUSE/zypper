#
# Makefile.am for solver/temporary
#

SUBDIRS =

INCLUDES = 					\
	-I$(top_srcdir) -Wall			\
	-I/usr/include/libxml2			\
	-DG_LOG_DOMAIN=\"devel.kkaempf\"

LDADD =	$(noinst_LTLIBRARIES) $(top_srcdir)/zypp/lib@PACKAGE@.la

noinst_PROGRAMS = rcparse

rcparse_SOURCES = rcparse.cc

noinst_HEADERS = \
	Buffer.h				\
	HelixExtract.h				\
	HelixParser.h				\
	HelixSourceImpl.h			\
	HelixPackageImpl.h

noinst_LTLIBRARIES =	lib@PACKAGE@_devel_kkaempf.la

lib@PACKAGE@_devel_kkaempf_la_SOURCES =		\
	Buffer.cc				\
	HelixExtract.cc				\
	HelixParser.cc				\
	HelixSourceImpl.cc			\
	HelixPackageImpl.cc

