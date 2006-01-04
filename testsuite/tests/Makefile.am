## Process this file with automake to produce Makefile.in
## ##################################################

# Do NOT list devel.* in subdirs. It's developers playgound and
# must not prevent anything from building.
SUBDIRS = 

## ##################################################

noinst_PROGRAMS = Arch Url1 Url2 Url3 Edition

## ##################################################

LDADD =	$(top_srcdir)/zypp/lib@PACKAGE@.la

## ##################################################

Arch_SOURCES = Arch.cc
Url1_SOURCES = Url1.cc
Url2_SOURCES = Url2.cc
Url3_SOURCES = Url3.cc
Edition_SOURCES = Edition.cc

## ##################################################

.PHONY:        always

$(noinst_PROGRAMS):    $(top_srcdir)/zypp/lib@PACKAGE@.la

$(top_srcdir)/zypp/lib@PACKAGE@.la:     always
	$(MAKE) -C $(top_srcdir)/zypp

## ##################################################
