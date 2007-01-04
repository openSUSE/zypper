INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(@PACKAGE@_helix_STAT_SRCS
Buffer.cc
XmlNode.cc
HelixExtract.cc
HelixParser.cc
HelixSourceImpl.cc
HelixLanguageImpl.cc
HelixPackageImpl.cc
HelixScriptImpl.cc
HelixMessageImpl.cc
HelixAtomImpl.cc
HelixPatchImpl.cc
HelixSelectionImpl.cc
HelixPatternImpl.cc
HelixProductImpl.cc
)

KDE3_AUTOMOC(${@PACKAGE@_helix_STAT_SRCS})

ADD_LIBRARY(@PACKAGE@_helix STATIC ${@PACKAGE@_helix_STAT_SRCS})


########### install files ###############




#original Makefile.am contents follow:

##
## Makefile.am for testsuite/src/helix
##
#
#INCLUDES = 					\
#	-I$(top_srcdir) -Wall			\
#	-I/usr/include/libxml2			\
#	-DG_LOG_DOMAIN=\"helix\"		\
#	-DZYPP_BASE_LOGGER_LOGGROUP=\"helix\"
#
#LDADD =	$(noinst_LTLIBRARIES) $(top_srcdir)/zypp/lib@PACKAGE@.la
#
#noinst_HEADERS = \
#	Buffer.h				\
#	XmlNode.h				\
#	HelixExtract.h				\
#	HelixParser.h				\
#	HelixSourceImpl.h			\
#	HelixPackageImpl.h			\
#	HelixLanguageImpl.h			\
#	HelixScriptImpl.h			\
#	HelixMessageImpl.h			\
#	HelixAtomImpl.h				\
#	HelixPatchImpl.h			\
#	HelixSelectionImpl.h			\
#	HelixPatternImpl.h			\
#	HelixProductImpl.h
#
#noinst_LTLIBRARIES =	lib@PACKAGE@_helix.la
#
#lib@PACKAGE@_helix_la_SOURCES =			\
#	Buffer.cc				\
#	XmlNode.cc				\
#	HelixExtract.cc				\
#	HelixParser.cc				\
#	HelixSourceImpl.cc			\
#	HelixLanguageImpl.cc			\
#	HelixPackageImpl.cc			\
#	HelixScriptImpl.cc			\
#	HelixMessageImpl.cc			\
#	HelixAtomImpl.cc			\
#	HelixPatchImpl.cc			\
#	HelixSelectionImpl.cc			\
#	HelixPatternImpl.cc			\
#	HelixProductImpl.cc
#
