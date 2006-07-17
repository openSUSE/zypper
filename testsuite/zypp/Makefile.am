## Process this file with automake to produce Makefile.in
## ##################################################

# Do NOT list devel.* in subdirs. It's developers playgound and
# must not prevent anything from building.
SUBDIRS =

## ##################################################

TESTS = Arch Url \
	RWPtr \
	Edition		\
	Capabilities	\
	PtrTypes \
	TranslatedText \
	CheckSum \
	PublicKey \
	PathInfo \
	Digest \
	Signature

check_PROGRAMS = Arch Url \
	RWPtr \
	Edition		\
	Capabilities	\
	PtrTypes \
	TranslatedText \
	CheckSum \
	PublicKey \
	PathInfo \
	Digest \
	Signature

## ##################################################

LDADD =	$(top_srcdir)/zypp/lib@PACKAGE@.la 

## ##################################################

Arch_SOURCES = Arch.cc
Arch_LDADD = $(LDADD) -lboost_unit_test_framework

TranslatedText_SOURCES  = TranslatedText.cc

PublicKey_SOURCES = PublicKey.cc
PublicKey_LDADD  = $(LDADD) -lboost_unit_test_framework

CheckSum_SOURCES = CheckSum.cc
CheckSum_LDADD  = $(LDADD) -lboost_unit_test_framework

RWPtr_SOURCES = RWPtr.cc

Edition_SOURCES = Edition.cc
Edition_LDADD  = $(LDADD) -lboost_unit_test_framework

Capabilities_SOURCES = Capabilities.cc
Capabilities_LDADD  = $(LDADD) -lboost_unit_test_framework

PtrTypes_SOURCES = PtrTypes.cc

PathInfo_SOURCES = PathInfo.cc
PathInfo_LDADD  = $(LDADD) -lboost_unit_test_framework

Digest_SOURCES = Digest.cc
Digest_LDADD  = $(LDADD) -lboost_unit_test_framework

Signature_SOURCES = Signature.cc
Signature_LDADD  = $(LDADD) -lboost_unit_test_framework

Url_SOURCES = Url.cc
Url_LDADD  = $(LDADD) -lboost_unit_test_framework


## ##################################################

.PHONY:        always

$(noinst_PROGRAMS):    $(top_srcdir)/zypp/lib@PACKAGE@.la

#$(top_srcdir)/zypp/lib@PACKAGE@.la:     always
#	$(MAKE) -C $(top_srcdir)/zypp

## ##################################################
