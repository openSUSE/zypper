## Process this file with automake to produce Makefile.in
## ##################################################

# Do NOT list devel.* in subdirs. It's developers playgound and
# must not prevent anything from building.
SUBDIRS =

## ##################################################

noinst_PROGRAMS = Arch Url1 Url2 Url3 Url4 Url5 Url \
	RWPtr \
	Edition		\
	Capabilities	\
	PtrTypes \
	TranslatedText \
	CheckSum \
	PublicKey

## ##################################################

LDADD =	$(top_srcdir)/zypp/lib@PACKAGE@.la -lboost_unit_test_framework

## ##################################################

Arch_SOURCES = Arch.cc
Url1_SOURCES = Url1.cc
Url2_SOURCES = Url2.cc
Url3_SOURCES = Url3.cc
Url4_SOURCES = Url4.cc
Url5_SOURCES = Url5.cc
Url_SOURCES  = Url.cc
TranslatedText_SOURCES  = TranslatedText.cc
PublicKey_SOURCES = PublicKey.cc
CheckSum_SOURCES = CheckSum.cc
RWPtr_SOURCES = RWPtr.cc
Edition_SOURCES = Edition.cc
Capabilities_SOURCES = Capabilities.cc
PtrTypes_SOURCES = PtrTypes.cc

## ##################################################

.PHONY:        always

$(noinst_PROGRAMS):    $(top_srcdir)/zypp/lib@PACKAGE@.la

#$(top_srcdir)/zypp/lib@PACKAGE@.la:     always
#	$(MAKE) -C $(top_srcdir)/zypp

## ##################################################
