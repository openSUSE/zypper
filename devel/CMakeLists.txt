INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(Example.createResolvable.bin_SRCS
Example.createResolvable.cc
)

KDE3_AUTOMOC(${Example.createResolvable.bin_SRCS})

KDE3_ADD_EXECUTABLE(Example.createResolvable.bin ${Example.createResolvable.bin_SRCS})

TARGET_LINK_LIBRARIES(Example.createResolvable.bin  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

SET(Example.createCapabilities.bin_SRCS
Example.createCapabilities.cc
)

KDE3_AUTOMOC(${Example.createCapabilities.bin_SRCS})

KDE3_ADD_EXECUTABLE(Example.createCapabilities.bin ${Example.createCapabilities.bin_SRCS})

TARGET_LINK_LIBRARIES(Example.createCapabilities.bin  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

SET(Example.COW_debug.bin_SRCS
Example.COW_debug.cc
)

KDE3_AUTOMOC(${Example.COW_debug.bin_SRCS})

KDE3_ADD_EXECUTABLE(Example.COW_debug.bin ${Example.COW_debug.bin_SRCS})

TARGET_LINK_LIBRARIES(Example.COW_debug.bin  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

SET(Example.EditionCompare.bin_SRCS
Example.EditionCompare.cc
)

KDE3_AUTOMOC(${Example.EditionCompare.bin_SRCS})

KDE3_ADD_EXECUTABLE(Example.EditionCompare.bin ${Example.EditionCompare.bin_SRCS})

TARGET_LINK_LIBRARIES(Example.EditionCompare.bin  ${QT_AND_KDECORE_LIBS} )


########### install files ###############




#original Makefile.am contents follow:

### Process this file with automake to produce Makefile.in
### ##################################################
#
## Do NOT list devel.* in subdirs. It's developers playgound
## and must not prevent anything from building.
#
#SUBDIRS =
#
### ##################################################
#
#noinst_PROGRAMS = \
#	Example.createResolvable.bin	\
#	Example.createCapabilities.bin	\
#	Example.COW_debug.bin		\
#	Example.EditionCompare.bin
#
### ##################################################
#
#LDADD =	$(top_srcdir)/zypp/lib@PACKAGE@.la
#
### ##################################################
#
#Example_createResolvable_bin_SOURCES 	= Example.createResolvable.cc
#Example_createCapabilities_bin_SOURCES 	= Example.createCapabilities.cc
#Example_COW_debug_bin_SOURCES 		= Example.COW_debug.cc
#Example_EditionCompare_bin_SOURCES	= Example.EditionCompare.cc
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
