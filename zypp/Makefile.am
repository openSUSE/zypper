## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS = base thread url media capability detail pool parser \
	source target solver zypp_detail

## ##################################################

pkginclude_HEADERS = NeedAType.h \
	Arch.h		\
	Bit.h		\
	ByteCount.h	\
	Capability.h	\
	CapFactory.h	\
	CapMatch.h	\
	CapSet.h	\
	CapSetFwd.h	\
	CapFilters.h	\
	CountryCode.h	\
	Date.h		\
	Dep.h		\
	Dependencies.h	\
	Edition.h	\
	LanguageCode.h	\
	Locale.h	\
	NVR.h		\
	NVRA.h		\
	NVRAD.h		\
	Range.h		\
	Rel.h		\
	RelCompare.h	\
	ResObject.h	\
	Resolvable.h	\
	ResTraits.h	\
	ResStatus.h	\
	ResStore.h	\
	ResFilters.h	\
	PoolItem.h	\
	ResPool.h	\
	ResPoolManager.h\
	Resolver.h	\
	ResolverProblem.h	\
	ProblemTypes.h	\
	ProblemSolution.h	\
	Package.h	\
	Pathname.h	\
	Source.h	\
	SourceFactory.h	\
	SourceManager.h	\
	SourceFeed.h	\
	Target.h	\
	Selection.h	\
	Pattern.h	\
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
	Callback.h	\
	DiskUsage.h	\
	CheckSum.h	\
	BaseVersion.h	\
	PatchRpm.h	\
	DeltaRpm.h	\
	ZYpp.h		\
	TranslatedText.h \
	ZYppFactory.h

## ##################################################

lib_LTLIBRARIES =	lib@PACKAGE@.la

## ##################################################

lib@PACKAGE@_la_SOURCES = \
	Arch.cc		\
	ByteCount.cc	\
	Capability.cc	\
	CapFactory.cc	\
	CapMatch.cc	\
	CapSet.cc	\
	CountryCode.cc	\
	Date.cc		\
	Dep.cc		\
	Dependencies.cc	\
	Edition.cc	\
	LanguageCode.cc	\
	Locale.cc	\
	NVR.cc		\
	NVRA.cc		\
	NVRAD.cc	\
	Range.cc	\
	Rel.cc		\
	ResObject.cc	\
	Resolvable.cc	\
	ResTraits.cc	\
	ResStatus.cc	\
	ResStore.cc	\
	PoolItem.cc	\
	ResPool.cc	\
	ResPoolManager.cc\
	Resolver.cc	\
	ResolverProblem.cc	\
	ProblemSolution.cc	\
	Package.cc	\
	Pathname.cc	\
	Source.cc	\
	SourceFactory.cc\
	SourceManager.cc\
	SourceFeed.cc	\
	Target.cc	\
	Selection.cc	\
	Pattern.cc	\
	Message.cc	\
	Script.cc	\
	Patch.cc	\
	Product.cc	\
	Changelog.cc	\
	ExternalProgram.cc\
	PathInfo.cc	\
	Digest.cc	\
	Url.cc		\
	TmpPath.cc	\
	DiskUsage.cc	\
	ZYpp.cc		\
	TranslatedText.cc \
	ZYppFactory.cc

lib@PACKAGE@_la_LDFLAGS =	@LIBZYPP_VERSION_INFO@

lib@PACKAGE@_la_LIBADD =        base/lib@PACKAGE@_base.la	\
				thread/lib@PACKAGE@_thread.la	\
				detail/lib@PACKAGE@_detail.la	\
				capability/lib@PACKAGE@_capability.la	\
				pool/lib@PACKAGE@_pool.la	\
				parser/lib@PACKAGE@_parser.la	\
				source/lib@PACKAGE@_source.la	\
				media/lib@PACKAGE@_media.la	\
				url/lib@PACKAGE@_url.la		\
				target/lib@PACKAGE@_target.la	\
				solver/lib@PACKAGE@_solver.la	\
				zypp_detail/lib@PACKAGE@_zypp_detail.la	\
				\
				-lutil

## ##################################################

EXTRA_DIST = @DOXYGEN/*

