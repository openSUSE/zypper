INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(Arch_SRCS
Arch.cc
)

KDE3_AUTOMOC(${Arch_SRCS})

IF(KDE3_BUILD_TESTS)

KDE3_ADD_EXECUTABLE(Arch ${Arch_SRCS})

TARGET_LINK_LIBRARIES(Arch  ${QT_AND_KDECORE_LIBS} boost_unit_test_framework )

ENDIF(KDE3_BUILD_TESTS)

########### next target ###############

SET(Url_SRCS
Url.cc
)

KDE3_AUTOMOC(${Url_SRCS})

IF(KDE3_BUILD_TESTS)

KDE3_ADD_EXECUTABLE(Url ${Url_SRCS})

TARGET_LINK_LIBRARIES(Url  ${QT_AND_KDECORE_LIBS} boost_unit_test_framework )

ENDIF(KDE3_BUILD_TESTS)

########### next target ###############

SET(RWPtr_SRCS
RWPtr.cc
)

KDE3_AUTOMOC(${RWPtr_SRCS})

IF(KDE3_BUILD_TESTS)

KDE3_ADD_EXECUTABLE(RWPtr ${RWPtr_SRCS})

TARGET_LINK_LIBRARIES(RWPtr  ${QT_AND_KDECORE_LIBS} )

ENDIF(KDE3_BUILD_TESTS)

########### next target ###############

SET(Edition_SRCS
Edition.cc
)

KDE3_AUTOMOC(${Edition_SRCS})

IF(KDE3_BUILD_TESTS)

KDE3_ADD_EXECUTABLE(Edition ${Edition_SRCS})

TARGET_LINK_LIBRARIES(Edition  ${QT_AND_KDECORE_LIBS} boost_unit_test_framework )

ENDIF(KDE3_BUILD_TESTS)

########### next target ###############

SET(Capabilities_SRCS
Capabilities.cc
)

KDE3_AUTOMOC(${Capabilities_SRCS})

IF(KDE3_BUILD_TESTS)

KDE3_ADD_EXECUTABLE(Capabilities ${Capabilities_SRCS})

TARGET_LINK_LIBRARIES(Capabilities  ${QT_AND_KDECORE_LIBS} boost_unit_test_framework )

ENDIF(KDE3_BUILD_TESTS)

########### next target ###############

SET(PtrTypes_SRCS
PtrTypes.cc
)

KDE3_AUTOMOC(${PtrTypes_SRCS})

IF(KDE3_BUILD_TESTS)

KDE3_ADD_EXECUTABLE(PtrTypes ${PtrTypes_SRCS})

TARGET_LINK_LIBRARIES(PtrTypes  ${QT_AND_KDECORE_LIBS} )

ENDIF(KDE3_BUILD_TESTS)

########### next target ###############

SET(TranslatedText_SRCS
TranslatedText.cc
)

KDE3_AUTOMOC(${TranslatedText_SRCS})

IF(KDE3_BUILD_TESTS)

KDE3_ADD_EXECUTABLE(TranslatedText ${TranslatedText_SRCS})

TARGET_LINK_LIBRARIES(TranslatedText  ${QT_AND_KDECORE_LIBS} )

ENDIF(KDE3_BUILD_TESTS)

########### next target ###############

SET(CheckSum_SRCS
CheckSum.cc
)

KDE3_AUTOMOC(${CheckSum_SRCS})

IF(KDE3_BUILD_TESTS)

KDE3_ADD_EXECUTABLE(CheckSum ${CheckSum_SRCS})

TARGET_LINK_LIBRARIES(CheckSum  ${QT_AND_KDECORE_LIBS} boost_unit_test_framework )

ENDIF(KDE3_BUILD_TESTS)

########### next target ###############

SET(PublicKey_SRCS
PublicKey.cc
)

KDE3_AUTOMOC(${PublicKey_SRCS})

IF(KDE3_BUILD_TESTS)

KDE3_ADD_EXECUTABLE(PublicKey ${PublicKey_SRCS})

TARGET_LINK_LIBRARIES(PublicKey  ${QT_AND_KDECORE_LIBS} boost_unit_test_framework )

ENDIF(KDE3_BUILD_TESTS)

########### next target ###############

SET(PathInfo_SRCS
PathInfo.cc
)

KDE3_AUTOMOC(${PathInfo_SRCS})

IF(KDE3_BUILD_TESTS)

KDE3_ADD_EXECUTABLE(PathInfo ${PathInfo_SRCS})

TARGET_LINK_LIBRARIES(PathInfo  ${QT_AND_KDECORE_LIBS} boost_unit_test_framework )

ENDIF(KDE3_BUILD_TESTS)

########### next target ###############

SET(Digest_SRCS
Digest.cc
)

KDE3_AUTOMOC(${Digest_SRCS})

IF(KDE3_BUILD_TESTS)

KDE3_ADD_EXECUTABLE(Digest ${Digest_SRCS})

TARGET_LINK_LIBRARIES(Digest  ${QT_AND_KDECORE_LIBS} boost_unit_test_framework )

ENDIF(KDE3_BUILD_TESTS)

########### next target ###############

SET(Signature_SRCS
Signature.cc
)

KDE3_AUTOMOC(${Signature_SRCS})

IF(KDE3_BUILD_TESTS)

KDE3_ADD_EXECUTABLE(Signature ${Signature_SRCS})

TARGET_LINK_LIBRARIES(Signature  ${QT_AND_KDECORE_LIBS} boost_unit_test_framework )

ENDIF(KDE3_BUILD_TESTS)

########### next target ###############

SET(KeyRing_SRCS
KeyRing.cc
)

KDE3_AUTOMOC(${KeyRing_SRCS})

IF(KDE3_BUILD_TESTS)

KDE3_ADD_EXECUTABLE(KeyRing ${KeyRing_SRCS})

TARGET_LINK_LIBRARIES(KeyRing  ${QT_AND_KDECORE_LIBS} boost_unit_test_framework )

ENDIF(KDE3_BUILD_TESTS)

########### next target ###############

SET(LazzyText_SRCS
LazzyText.cc
)

KDE3_AUTOMOC(${LazzyText_SRCS})

IF(KDE3_BUILD_TESTS)

KDE3_ADD_EXECUTABLE(LazzyText ${LazzyText_SRCS})

TARGET_LINK_LIBRARIES(LazzyText  ${QT_AND_KDECORE_LIBS} boost_unit_test_framework )

ENDIF(KDE3_BUILD_TESTS)

########### install files ###############




#original Makefile.am contents follow:

### Process this file with automake to produce Makefile.in
### ##################################################
#
## Do NOT list devel.* in subdirs. It's developers playgound and
## must not prevent anything from building.
#SUBDIRS =
#
### ##################################################
#
#TESTS = Arch Url \
#	RWPtr \
#	Edition		\
#	Capabilities	\
#	PtrTypes \
#	TranslatedText \
#	CheckSum \
#	PublicKey \
#	PathInfo \
#	Digest \
#	Signature \
#	KeyRing \
#	LazzyText
#
#check_PROGRAMS = Arch Url \
#	RWPtr \
#	Edition		\
#	Capabilities	\
#	PtrTypes \
#	TranslatedText \
#	CheckSum \
#	PublicKey \
#	PathInfo \
#	Digest \
#	Signature \
#	KeyRing \
#	LazzyText
#
### ##################################################
#
#LDADD =	$(top_srcdir)/zypp/lib@PACKAGE@.la 
#
### ##################################################
#
#Arch_SOURCES = Arch.cc
#Arch_LDADD = $(LDADD) -lboost_unit_test_framework
#
#TranslatedText_SOURCES  = TranslatedText.cc
#
#PublicKey_SOURCES = PublicKey.cc
#PublicKey_LDADD  = $(LDADD) -lboost_unit_test_framework
#
#CheckSum_SOURCES = CheckSum.cc
#CheckSum_LDADD  = $(LDADD) -lboost_unit_test_framework
#
#RWPtr_SOURCES = RWPtr.cc
#
#Edition_SOURCES = Edition.cc
#Edition_LDADD  = $(LDADD) -lboost_unit_test_framework
#
#Capabilities_SOURCES = Capabilities.cc
#Capabilities_LDADD  = $(LDADD) -lboost_unit_test_framework
#
#PtrTypes_SOURCES = PtrTypes.cc
#
#PathInfo_SOURCES = PathInfo.cc
#PathInfo_LDADD  = $(LDADD) -lboost_unit_test_framework
#
#Digest_SOURCES = Digest.cc
#Digest_LDADD  = $(LDADD) -lboost_unit_test_framework
#
#Signature_SOURCES = Signature.cc
#Signature_LDADD  = $(LDADD) -lboost_unit_test_framework
#
#Url_SOURCES = Url.cc
#Url_LDADD  = $(LDADD) -lboost_unit_test_framework
#
#KeyRing_SOURCES = KeyRing.cc
#KeyRing_LDADD  = $(LDADD) -lboost_unit_test_framework
#
#LazzyText_SOURCES = LazzyText.cc
#LazzyText_LDADD  = $(LDADD) $(top_srcdir)/zypp2/lib@PACKAGE@2.la  -lboost_unit_test_framework
#
### ##################################################
#
#.PHONY:        always
#
#$(noinst_PROGRAMS):    $(top_srcdir)/zypp/lib@PACKAGE@.la
#
##$(top_srcdir)/zypp/lib@PACKAGE@.la:     always
##	$(MAKE) -C $(top_srcdir)/zypp
#
### ##################################################
