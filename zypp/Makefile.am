## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS = base thread url media capability detail pool parser \
	source target solver zypp_detail ui

AM_CXXFLAGS = -DZYPP_BASE_LOGGER_LOGGROUP=\"zypp\"

## ##################################################

pkginclude_HEADERS = NeedAType.h \
	Arch.h		\
	Atom.h		\
	Bit.h		\
	ByteCount.h	\
	CapAndItem.h	\
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
	Language.h	\
	LanguageCode.h	\
	Locale.h	\
	NVR.h		\
	NVRA.h		\
	NVRAD.h		\
	Range.h		\
	Rel.h		\
	RelCompare.h	\
	ResObject.h	\
	SystemResObject.h	\
	Resolvable.h	\
	ResTraits.h	\
	ResStatus.h	\
	ResStore.h	\
	ResFilters.h	\
	PoolItem.h	\
	ResPool.h	\
	NameKindProxy.h	\
	ResPoolProxy.h	\
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
	SourceCache.h	\
	SourceFeed.h	\
	SrcPackage.h	\
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
	DiskUsageCounter.h	\
	CheckSum.h	\
	BaseVersion.h	\
	PatchRpm.h	\
	DeltaRpm.h	\
	VendorAttr.h	\
	UpgradeStatistics.h	\
	TranslatedText.h \
	ZYpp.h		\
	ZYppCommit.h		\
	ZYppCommitPolicy.h	\
	ZYppCommitResult.h	\
	ZYppFactory.h	\
	ZYppCallbacks.h	\
	SilentCallbacks.h \
	KeyRing.h

## ##################################################

lib_LTLIBRARIES =	lib@PACKAGE@.la

## ##################################################

lib@PACKAGE@_la_SOURCES = \
	LanguageCode.cc	\
	Arch.cc		\
	Atom.cc		\
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
	Language.cc	\
	Locale.cc	\
	NVR.cc		\
	NVRA.cc		\
	NVRAD.cc	\
	Range.cc	\
	Rel.cc		\
	ResObject.cc	\
	SystemResObject.cc	\
	Resolvable.cc	\
	ResTraits.cc	\
	ResStatus.cc	\
	ResStore.cc	\
	PoolItem.cc	\
	ResPool.cc	\
	NameKindProxy.cc	\
	ResPoolProxy.cc	\
	ResPoolManager.cc\
	Resolver.cc	\
	ResolverProblem.cc	\
	ProblemSolution.cc	\
	Package.cc	\
	Pathname.cc	\
	Source.cc	\
	SourceFactory.cc\
	SourceManager.cc\
	SourceCache.cc	\
	SourceFeed.cc	\
	SrcPackage.cc	\
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
	DiskUsageCounter.cc	\
	VendorAttr.cc	\
	UpgradeStatistics.cc	\
	ZYpp.cc		\
	ZYppCommitPolicy.cc	\
	ZYppCommitResult.cc	\
	TranslatedText.cc \
	ZYppFactory.cc \
	KeyRing.cc


lib@PACKAGE@_la_LDFLAGS =	@LIBZYPP_VERSION_INFO@

lib@PACKAGE@_la_LIBADD =        thread/lib@PACKAGE@_thread.la		\
				detail/lib@PACKAGE@_detail.la		\
				capability/lib@PACKAGE@_capability.la	\
				pool/lib@PACKAGE@_pool.la		\
				parser/lib@PACKAGE@_parser.la		\
				source/lib@PACKAGE@_source.la		\
				media/lib@PACKAGE@_media.la		\
				url/lib@PACKAGE@_url.la			\
				target/lib@PACKAGE@_target.la		\
				solver/lib@PACKAGE@_solver.la		\
				zypp_detail/lib@PACKAGE@_zypp_detail.la	\
				ui/lib@PACKAGE@_ui.la			\
				\
				base/lib@PACKAGE@_base.la		\
				-lboost_filesystem			\
				-lutil
# !!! base/lib@PACKAGE@_base.la MUST BE THE LAST zypp lib linked

## ##################################################

EXTRA_DIST = @DOXYGEN/*

#<INSTALL-HEADER-TARGET>

# Automatically inserted by insert_install_target
# DO NOT REMOVE THE TAGS!

# Install header files only if different to version already installed -
# prevent unnecessary installations to keep make dependency changes
# and thus rebuilds for dependent packages as low as possible.

install-pkgincludeHEADERS: $(pkginclude_HEADERS)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(pkgincludedir)
	@list='$(pkginclude_HEADERS)';							\
	for p in $$list; do								\
		if test -f "$$p"; then d= ; else d="$(srcdir)/"; fi; 			\
		if ! cmp -s $$d$$p $(DESTDIR)$(pkgincludedir)/$$p; then			\
			echo " $(INSTALL_DATA) $$d$$p $(DESTDIR)$(pkgincludedir)/$$p";	\
			$(INSTALL_DATA) $$d$$p $(DESTDIR)$(pkgincludedir)/$$p;		\
	  	fi									\
	done

#</INSTALL-HEADER-TARGET>

