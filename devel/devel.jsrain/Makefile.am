## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS = yum

noinst_PROGRAMS =	Msg Msg.debug Patch Patch.debug Script Script.debug PatchSelect PatchSelect.debug PatchRead PatchRead.debug SourceRead SourceRead.debug SourceCacheRead SourceCacheRead.debug SourceProvide SourceProvide.debug RpmTarget RpmTarget.debug RpmInstall RpmInstall.debug Target Target.debug


## ##################################################

INCLUDES =

AM_LDFLAGS =

AM_CXXFLAGS =	-pg

LDADD =		$(top_srcdir)/zypp/lib@PACKAGE@.la

## ##################################################

Msg_SOURCES = 		Msg.cc

Msg_debug_SOURCES = 	$(Msg_SOURCES)
Msg_debug_LDFLAGS =	-static

Patch_SOURCES = 	Patch.cc

Patch_debug_SOURCES = 	$(Patch_SOURCES)
Patch_debug_LDFLAGS =	-static

Script_SOURCES = 	Script.cc

Script_debug_SOURCES = 	$(Script_SOURCES)
Script_debug_LDFLAGS =	-static

PatchSelect_SOURCES = 	PatchSelect.cc

PatchSelect_debug_SOURCES = 	$(PatchSelect_SOURCES)
PatchSelect_debug_LDFLAGS =	-static

PatchRead_SOURCES = 	PatchRead.cc

PatchRead_debug_SOURCES = 	$(PatchRead_SOURCES)
PatchRead_debug_LDFLAGS =	-static

SourceRead_SOURCES = 		SourceRead.cc

SourceRead_debug_SOURCES = 	$(SourceRead_SOURCES)
SourceRead_debug_LDFLAGS =	-static

SourceCacheRead_SOURCES = 		SourceCacheRead.cc

SourceCacheRead_debug_SOURCES = 	$(SourceCacheRead_SOURCES)
SourceCacheRead_debug_LDFLAGS =	-static

SourceProvide_SOURCES = 	SourceProvide.cc

SourceProvide_debug_SOURCES = 	$(SourceProvide_SOURCES)
SourceProvide_debug_LDFLAGS =	-static

RpmTarget_SOURCES = 		RpmTarget.cc

RpmTarget_debug_SOURCES = 	$(RpmTarget_SOURCES)
RpmTarget_debug_LDFLAGS =	-static

RpmInstall_SOURCES = 		RpmInstall.cc

RpmInstall_debug_SOURCES = 	$(RpmInstall_SOURCES)
RpmInstall_debug_LDFLAGS =	-static

Target_SOURCES = 		Target.cc

Target_debug_SOURCES = 	$(Target_SOURCES)
Target_debug_LDFLAGS =	-static



## ##################################################

.PHONY:        always

$(noinst_PROGRAMS):    $(top_srcdir)/zypp/lib@PACKAGE@.la

$(top_srcdir)/zypp/lib@PACKAGE@.la:     always
	$(MAKE) -C $(top_srcdir)/zypp

## ##################################################

misc: genclass always
	chmod 750 genclass
