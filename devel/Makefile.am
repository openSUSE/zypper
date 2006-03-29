## Process this file with automake to produce Makefile.in
## ##################################################

# Do NOT list devel.* in subdirs. It's developers playgound
# and must not prevent anything from building.

SUBDIRS =

## ##################################################

noinst_PROGRAMS = \
	Example.createResolvable.bin	\
	Example.createCapabilities.bin	\
	Example.COW_debug.bin		\
	Example.EditionCompare.bin

## ##################################################

LDADD =	$(top_srcdir)/zypp/lib@PACKAGE@.la

## ##################################################

Example_createResolvable_bin_SOURCES 	= Example.createResolvable.cc
Example_createCapabilities_bin_SOURCES 	= Example.createCapabilities.cc
Example_COW_debug_bin_SOURCES 		= Example.COW_debug.cc
Example_EditionCompare_bin_SOURCES	= Example.EditionCompare.cc

## ##################################################

.PHONY:        always

$(noinst_PROGRAMS):    $(top_srcdir)/zypp/lib@PACKAGE@.la

$(top_srcdir)/zypp/lib@PACKAGE@.la:     always
	$(MAKE) -C $(top_srcdir)/zypp

## ##################################################
