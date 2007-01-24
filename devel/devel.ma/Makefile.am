## Process this file with automake to produce Makefile.in
## ##################################################

noinst_PROGRAMS = AOUT Prof

## ##################################################

INCLUDES =

AM_LDFLAGS =

AM_CXXFLAGS = -pg

LDADD =	      -L$(top_srcdir)/zypp/.libs -lzypp -lboost_regex
# -lcwd

## ##################################################

ADD_SOURCES =

#		debug/Debug.cc

## ##################################################

AOUT_SOURCES = $(ADD_SOURCES)
AOUT_DEPENDENCIES =  $(top_srcdir)/zypp/lib@PACKAGE@.la
AOUT_LDFLAGS = -static

## ##################################################

%: %.cc $(AOUT_DEPENDENCIES) $(AOUT_OBJECTS)
	if $(CXX) \
	$(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) \
	-MT $*.o -MD -MP -MF "$(DEPDIR)/$*.Tpo" -c -o $*.o $<; \
	then mv -f "$(DEPDIR)/$*.Tpo" "$(DEPDIR)/$*.Po"; else rm -f "$(DEPDIR)/$*.Tpo"; exit 1; fi
	rm -f $@
	$(CXXLINK) $(AOUT_LDFLAGS) $*.o $(AOUT_OBJECTS) $(AOUT_LDADD) $(LIBS)

## ##################################################

PROF_OBJ = 	$(strip $(shell find $(top_srcdir)/zypp -name '*.o' | grep -v 'LogControl.o') ) \
		$(top_srcdir)/zypp/base/.libs/LogControl.o

Prof_SOURCES = Prof.cc
Prof_LDADD   = -lboost_regex -lboost_filesystem -lcurl -lidn -lblkid -luuid -lxml2 -lrpm -lutil
Prof_LDFLAGS = $(PROF_OBJ)

## ##################################################

.PHONY:		always
$(top_srcdir)/zypp/lib@PACKAGE@.la:     always
	$(MAKE) -C $(top_srcdir)/zypp

xtp:
	echo $(PROF_OBJ)

## ##################################################
