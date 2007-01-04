ADD_SUBDIRECTORY( yum ) 

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(Msg_SRCS
Msg.cc
)

KDE3_AUTOMOC(${Msg_SRCS})

KDE3_ADD_EXECUTABLE(Msg ${Msg_SRCS})

TARGET_LINK_LIBRARIES(Msg  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

SET(Msg.debug_SRCS
$(Msg_SOURCES)
)

KDE3_AUTOMOC(${Msg.debug_SRCS})

KDE3_ADD_EXECUTABLE(Msg.debug ${Msg.debug_SRCS})

TARGET_LINK_LIBRARIES(Msg.debug  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

SET(Patch_SRCS
Patch.cc
)

KDE3_AUTOMOC(${Patch_SRCS})

KDE3_ADD_EXECUTABLE(Patch ${Patch_SRCS})

TARGET_LINK_LIBRARIES(Patch  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

SET(Patch.debug_SRCS
$(Patch_SOURCES)
)

KDE3_AUTOMOC(${Patch.debug_SRCS})

KDE3_ADD_EXECUTABLE(Patch.debug ${Patch.debug_SRCS})

TARGET_LINK_LIBRARIES(Patch.debug  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

SET(Script_SRCS
Script.cc
)

KDE3_AUTOMOC(${Script_SRCS})

KDE3_ADD_EXECUTABLE(Script ${Script_SRCS})

TARGET_LINK_LIBRARIES(Script  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

SET(Script.debug_SRCS
$(Script_SOURCES)
)

KDE3_AUTOMOC(${Script.debug_SRCS})

KDE3_ADD_EXECUTABLE(Script.debug ${Script.debug_SRCS})

TARGET_LINK_LIBRARIES(Script.debug  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

SET(PatchSelect_SRCS
PatchSelect.cc
)

KDE3_AUTOMOC(${PatchSelect_SRCS})

KDE3_ADD_EXECUTABLE(PatchSelect ${PatchSelect_SRCS})

TARGET_LINK_LIBRARIES(PatchSelect  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

SET(PatchSelect.debug_SRCS
$(PatchSelect_SOURCES)
)

KDE3_AUTOMOC(${PatchSelect.debug_SRCS})

KDE3_ADD_EXECUTABLE(PatchSelect.debug ${PatchSelect.debug_SRCS})

TARGET_LINK_LIBRARIES(PatchSelect.debug  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

SET(PatchRead_SRCS
PatchRead.cc
)

KDE3_AUTOMOC(${PatchRead_SRCS})

KDE3_ADD_EXECUTABLE(PatchRead ${PatchRead_SRCS})

TARGET_LINK_LIBRARIES(PatchRead  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

SET(PatchRead.debug_SRCS
$(PatchRead_SOURCES)
)

KDE3_AUTOMOC(${PatchRead.debug_SRCS})

KDE3_ADD_EXECUTABLE(PatchRead.debug ${PatchRead.debug_SRCS})

TARGET_LINK_LIBRARIES(PatchRead.debug  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

SET(SourceRead_SRCS
SourceRead.cc
)

KDE3_AUTOMOC(${SourceRead_SRCS})

KDE3_ADD_EXECUTABLE(SourceRead ${SourceRead_SRCS})

TARGET_LINK_LIBRARIES(SourceRead  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

SET(SourceRead.debug_SRCS
$(SourceRead_SOURCES)
)

KDE3_AUTOMOC(${SourceRead.debug_SRCS})

KDE3_ADD_EXECUTABLE(SourceRead.debug ${SourceRead.debug_SRCS})

TARGET_LINK_LIBRARIES(SourceRead.debug  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

SET(SourceCacheRead_SRCS
SourceCacheRead.cc
)

KDE3_AUTOMOC(${SourceCacheRead_SRCS})

KDE3_ADD_EXECUTABLE(SourceCacheRead ${SourceCacheRead_SRCS})

TARGET_LINK_LIBRARIES(SourceCacheRead  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

SET(SourceCacheRead.debug_SRCS
$(SourceCacheRead_SOURCES)
)

KDE3_AUTOMOC(${SourceCacheRead.debug_SRCS})

KDE3_ADD_EXECUTABLE(SourceCacheRead.debug ${SourceCacheRead.debug_SRCS})

TARGET_LINK_LIBRARIES(SourceCacheRead.debug  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

SET(SourceProvide_SRCS
SourceProvide.cc
)

KDE3_AUTOMOC(${SourceProvide_SRCS})

KDE3_ADD_EXECUTABLE(SourceProvide ${SourceProvide_SRCS})

TARGET_LINK_LIBRARIES(SourceProvide  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

SET(SourceProvide.debug_SRCS
$(SourceProvide_SOURCES)
)

KDE3_AUTOMOC(${SourceProvide.debug_SRCS})

KDE3_ADD_EXECUTABLE(SourceProvide.debug ${SourceProvide.debug_SRCS})

TARGET_LINK_LIBRARIES(SourceProvide.debug  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

SET(RpmTarget_SRCS
RpmTarget.cc
)

KDE3_AUTOMOC(${RpmTarget_SRCS})

KDE3_ADD_EXECUTABLE(RpmTarget ${RpmTarget_SRCS})

TARGET_LINK_LIBRARIES(RpmTarget  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

SET(RpmTarget.debug_SRCS
$(RpmTarget_SOURCES)
)

KDE3_AUTOMOC(${RpmTarget.debug_SRCS})

KDE3_ADD_EXECUTABLE(RpmTarget.debug ${RpmTarget.debug_SRCS})

TARGET_LINK_LIBRARIES(RpmTarget.debug  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

SET(RpmInstall_SRCS
RpmInstall.cc
)

KDE3_AUTOMOC(${RpmInstall_SRCS})

KDE3_ADD_EXECUTABLE(RpmInstall ${RpmInstall_SRCS})

TARGET_LINK_LIBRARIES(RpmInstall  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

SET(RpmInstall.debug_SRCS
$(RpmInstall_SOURCES)
)

KDE3_AUTOMOC(${RpmInstall.debug_SRCS})

KDE3_ADD_EXECUTABLE(RpmInstall.debug ${RpmInstall.debug_SRCS})

TARGET_LINK_LIBRARIES(RpmInstall.debug  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

SET(Target_SRCS
Target.cc
)

KDE3_AUTOMOC(${Target_SRCS})

KDE3_ADD_EXECUTABLE(Target ${Target_SRCS})

TARGET_LINK_LIBRARIES(Target  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

SET(Target.debug_SRCS
$(Target_SOURCES)
)

KDE3_AUTOMOC(${Target.debug_SRCS})

KDE3_ADD_EXECUTABLE(Target.debug ${Target.debug_SRCS})

TARGET_LINK_LIBRARIES(Target.debug  ${QT_AND_KDECORE_LIBS} )


########### install files ###############




#original Makefile.am contents follow:

### Process this file with automake to produce Makefile.in
### ##################################################
#
#SUBDIRS = yum
#
#noinst_PROGRAMS =	Msg Msg.debug Patch Patch.debug Script Script.debug PatchSelect PatchSelect.debug PatchRead PatchRead.debug SourceRead SourceRead.debug SourceCacheRead SourceCacheRead.debug SourceProvide SourceProvide.debug RpmTarget RpmTarget.debug RpmInstall RpmInstall.debug Target Target.debug
#
#
### ##################################################
#
#INCLUDES =
#
#AM_LDFLAGS =
#
#AM_CXXFLAGS =	-pg
#
#LDADD =		$(top_srcdir)/zypp/lib@PACKAGE@.la
#
### ##################################################
#
#Msg_SOURCES = 		Msg.cc
#
#Msg_debug_SOURCES = 	$(Msg_SOURCES)
#Msg_debug_LDFLAGS =	-static
#
#Patch_SOURCES = 	Patch.cc
#
#Patch_debug_SOURCES = 	$(Patch_SOURCES)
#Patch_debug_LDFLAGS =	-static
#
#Script_SOURCES = 	Script.cc
#
#Script_debug_SOURCES = 	$(Script_SOURCES)
#Script_debug_LDFLAGS =	-static
#
#PatchSelect_SOURCES = 	PatchSelect.cc
#
#PatchSelect_debug_SOURCES = 	$(PatchSelect_SOURCES)
#PatchSelect_debug_LDFLAGS =	-static
#
#PatchRead_SOURCES = 	PatchRead.cc
#
#PatchRead_debug_SOURCES = 	$(PatchRead_SOURCES)
#PatchRead_debug_LDFLAGS =	-static
#
#SourceRead_SOURCES = 		SourceRead.cc
#
#SourceRead_debug_SOURCES = 	$(SourceRead_SOURCES)
#SourceRead_debug_LDFLAGS =	-static
#
#SourceCacheRead_SOURCES = 		SourceCacheRead.cc
#
#SourceCacheRead_debug_SOURCES = 	$(SourceCacheRead_SOURCES)
#SourceCacheRead_debug_LDFLAGS =	-static
#
#SourceProvide_SOURCES = 	SourceProvide.cc
#
#SourceProvide_debug_SOURCES = 	$(SourceProvide_SOURCES)
#SourceProvide_debug_LDFLAGS =	-static
#
#RpmTarget_SOURCES = 		RpmTarget.cc
#
#RpmTarget_debug_SOURCES = 	$(RpmTarget_SOURCES)
#RpmTarget_debug_LDFLAGS =	-static
#
#RpmInstall_SOURCES = 		RpmInstall.cc
#
#RpmInstall_debug_SOURCES = 	$(RpmInstall_SOURCES)
#RpmInstall_debug_LDFLAGS =	-static
#
#Target_SOURCES = 		Target.cc
#
#Target_debug_SOURCES = 	$(Target_SOURCES)
#Target_debug_LDFLAGS =	-static
#
#
#
### ##################################################
#
#.PHONY:        always
#
#$(noinst_PROGRAMS):    $(top_srcdir)/zypp/lib@PACKAGE@.la
#
#$(top_srcdir)/zypp/lib@PACKAGE@.la:     always
#	$(MAKE) -C $(top_srcdir)/zypp
#
### ##################################################
#
#misc: genclass always
#	chmod 750 genclass
