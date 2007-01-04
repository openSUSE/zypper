INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(deptestomatic.multi_SRCS
deptestomatic.cc
KeyRingCallbacks.h
)

KDE3_AUTOMOC(${deptestomatic.multi_SRCS})

KDE3_ADD_EXECUTABLE(deptestomatic.multi ${deptestomatic.multi_SRCS})

TARGET_LINK_LIBRARIES(deptestomatic.multi  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

SET(helixparse_SRCS
helixparse.cc
)

KDE3_AUTOMOC(${helixparse_SRCS})

KDE3_ADD_EXECUTABLE(helixparse ${helixparse_SRCS})

TARGET_LINK_LIBRARIES(helixparse  ${QT_AND_KDECORE_LIBS} )


########### install files ###############




#original Makefile.am contents follow:

##
## Makefile.am for zypp/testsuite/solver/src
##
#
#INCLUDES = -I/usr/include/libxml2	\
#	-Ihelix				\
#	-DZYPP_BASE_LOGGER_LOGGROUP=\"testsuite\"
#
#LDADD =	$(top_srcdir)/zypp/lib@PACKAGE@.la $(top_srcdir)/testsuite/src/helix/lib@PACKAGE@_helix.la
#
## We're not ready for this yet.
##check_PROGRAMS = deptestomatic.multi
#noinst_PROGRAMS = deptestomatic.multi			\
#		helixparse
#
#deptestomatic_multi_SOURCES =				\
#	deptestomatic.cc KeyRingCallbacks.h
#
#helixparse_SOURCES =					\
#	helixparse.cc
