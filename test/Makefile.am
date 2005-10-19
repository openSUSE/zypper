## Process this file with automake to produce Makefile.in
## ##################################################

noinst_PROGRAMS =	Main Main.debug Msg Msg.debug Patch Patch.debug Script Script.debug

## ##################################################

INCLUDES =

AM_LDFLAGS =

AM_CXXFLAGS =	-pg

LDADD =		$(top_srcdir)/zypp/lib@PACKAGE@.la

## ##################################################

Main_SOURCES = 		Main.cc

Main_debug_SOURCES = 	$(Main_SOURCES)
Main_debug_LDFLAGS =	-static

Msg_SOURCES = 		Msg.cc

Msg_debug_SOURCES = 	$(Msg_SOURCES)
Msg_debug_LDFLAGS =	-static

Patch_SOURCES = 	Patch.cc

Patch_debug_SOURCES = 	$(Patch_SOURCES)
Patch_debug_LDFLAGS =	-static

Script_SOURCES = 	Script.cc

Script_debug_SOURCES = 	$(Script_SOURCES)
Script_debug_LDFLAGS =	-static

## ##################################################

.PHONY:        always

$(noinst_PROGRAMS):    $(top_srcdir)/zypp/lib@PACKAGE@.la

$(top_srcdir)/zypp/lib@PACKAGE@.la:     always
	$(MAKE) -C $(top_srcdir)/zypp

## ##################################################

misc: genclass always
	chmod 750 genclass
