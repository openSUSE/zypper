INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(AOUT_SRCS
AOUT.cc
$(ADD_SOURCES)
)

KDE3_AUTOMOC(${AOUT_SRCS})

KDE3_ADD_EXECUTABLE(AOUT ${AOUT_SRCS})

TARGET_LINK_LIBRARIES(AOUT  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

SET(Prof_SRCS
Prof.cc
)

KDE3_AUTOMOC(${Prof_SRCS})

KDE3_ADD_EXECUTABLE(Prof ${Prof_SRCS})

TARGET_LINK_LIBRARIES(Prof  ${QT_AND_KDECORE_LIBS} boost_regex boost_filesystem curl idn blkid uuid xml2 rpm util )


########### install files ###############




#original Makefile.am contents follow:

### Process this file with automake to produce Makefile.in
### ##################################################
#
#noinst_PROGRAMS = AOUT Prof
#
### ##################################################
#
#INCLUDES =
#
#AM_LDFLAGS =
#
#AM_CXXFLAGS = -pg
#
#LDADD =		-L$(top_srcdir)/zypp/.libs -lzypp -lboost_regex
#
### ##################################################
#
#ADD_SOURCES = 	xml/Node.cc		xml/Node.h	\
#		xml/Reader.cc		xml/Reader.h	\
#		xml/XmlString.cc	xml/XmlString.h	\
#		xml/libxmlfwd.cc	xml/libxmlfwd.h
#
### ##################################################
#
#AOUT_SOURCES = AOUT.cc $(ADD_SOURCES)
#AOUT_LDFLAGS = -static
#
### ##################################################
#
#%: %.cc $(top_srcdir)/zypp/lib@PACKAGE@.la
#	touch .deps/$@.Po
#	ln -sf $@.Po .deps/AOUT.Po
#	ln -sf $@.cc AOUT.cc
#	ln -sf $@.o  AOUT.o
#	$(MAKE) AOUT
#	mv AOUT $@
#
### ##################################################
#
#PROF_OBJ = 	$(strip $(shell find $(top_srcdir)/zypp -name '*.o' | grep -v 'LogControl.o') ) \
#		$(top_srcdir)/zypp/base/.libs/LogControl.o
#
#Prof_SOURCES = Prof.cc
#Prof_LDADD   = -lboost_regex -lboost_filesystem -lcurl -lidn -lblkid -luuid -lxml2 -lrpm -lutil
#Prof_LDFLAGS = $(PROF_OBJ)
#
### ##################################################
#
#.PHONY:		always
#
#Tools.h:	$(top_srcdir)/zypp/lib@PACKAGE@.la
#
#$(top_srcdir)/zypp/lib@PACKAGE@.la:     always
#	$(MAKE) -C $(top_srcdir)/zypp
#
#xtp:
#	echo $(PROF_OBJ)
#
### ##################################################
