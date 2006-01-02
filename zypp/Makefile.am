## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS = base parser capability solver source detail media url

## ##################################################

include_HEADERS = NeedAType.h \
	Arch.h		\
	ByteCount.h	\
	Capability.h	\
	CapFactory.h	\
	CapSet.h	\
	CountryCode.h	\
	Date.h		\
	Dependencies.h	\
	Edition.h	\
	LanguageCode.h	\
	Locale.h	\
	NVRA.h		\
	Rel.h		\
	ResObject.h	\
	Resolvable.h	\
	ResTraits.h	\
	Package.h	\
	Pathname.h	\
	Selection.h	\
	Message.h	\
	Script.h	\
	Patch.h		\
	Product.h	\
	Changelog.h	\
	ExternalProgram.h\
	Pathname.h	\
	PathInfo.h	\
	Digest.h	\
	Url.h		\
	TmpPath.h	\
	KVMap.h		\
	Callback.h

## ##################################################

lib_LTLIBRARIES =	lib@PACKAGE@.la

## ##################################################

lib@PACKAGE@_la_SOURCES = \
	Arch.cc		\
	ByteCount.cc	\
	Capability.cc	\
	CapFactory.cc	\
	CapSet.cc	\
	CountryCode.cc	\
	Date.cc		\
	Dependencies.cc	\
	Edition.cc	\
	LanguageCode.cc	\
	Locale.cc	\
	NVRA.cc		\
	Rel.cc		\
	ResObject.cc	\
	Resolvable.cc	\
	ResTraits.cc	\
	Package.cc	\
	Pathname.cc	\
	Selection.cc	\
	Message.cc	\
	Script.cc	\
	Patch.cc	\
	Product.cc	\
	Changelog.cc	\
	ExternalProgram.cc\
	PathInfo.cc	\
	Digest.cc	\
	Url.cc		\
	TmpPath.cc

lib@PACKAGE@_la_LDFLAGS =	@LIB_VERSION_INFO@

lib@PACKAGE@_la_LIBADD =        base/lib@PACKAGE@_base.la	\
				detail/lib@PACKAGE@_detail.la	\
				capability/lib@PACKAGE@_capability.la	\
				solver/lib@PACKAGE@_solver.la	\
				parser/lib@PACKAGE@_parser.la	\
				source/lib@PACKAGE@_source.la	\
				media/lib@PACKAGE@_media.la	\
				url/lib@PACKAGE@_url.la	\
				-lutil

## ##################################################
