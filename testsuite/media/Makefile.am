## Process this file with automake to produce Makefile.in
## ##################################################

# Do NOT list devel.* in subdirs. It's developers playgound and
# must not prevent anything from building.
SUBDIRS =

## ##################################################


TESTS = media1 fileexists

noinst_PROGRAMS = media2 media3 media4 fileexists

check_PROGRAMS = media1 fileexists


## ##################################################

LDADD =	$(top_srcdir)/zypp/lib@PACKAGE@.la 

## ##################################################

media1_SOURCES = media1.cc mymediaverifier.h
media1_LDADD = $(LDADD) -lboost_unit_test_framework

fileexists_SOURCES = file_exists_test.cc mymediaverifier.h
fileexists_LDADD = $(LDADD) -lboost_unit_test_framework

media2_SOURCES = media2.cc mymediaverifier.h
media2_LDADD = $(LDADD)

media3_SOURCES = media3.cc mymediaverifier.h
media3_LDADD = $(LDADD)

media4_SOURCES = media4.cc mymediaverifier.h
media4_LDADD = $(LDADD)

## ##################################################

.PHONY:        always

$(noinst_PROGRAMS):    $(top_srcdir)/zypp/lib@PACKAGE@.la

#$(top_srcdir)/zypp/lib@PACKAGE@.la:     always
#	$(MAKE) -C $(top_srcdir)/zypp

## ##################################################