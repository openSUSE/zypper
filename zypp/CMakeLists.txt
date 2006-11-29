####################################################################
# INCLUDES                                                         #
####################################################################

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

####################################################################
# SOURCES                                                          #
####################################################################

SET( zypp_SRCS
LanguageCode.cc
Arch.cc
Atom.cc
ByteCount.cc
Capability.cc
CapFactory.cc
CapMatch.cc
CapSet.cc
CheckSum.cc
CountryCode.cc
Date.cc
Dep.cc
Dependencies.cc
Edition.cc
Language.cc
Locale.cc
NVR.cc
NVRA.cc
NVRAD.cc
Range.cc
Rel.cc
ResObject.cc
SystemResObject.cc
Resolvable.cc
ResTraits.cc
ResStatus.cc
ResStore.cc
PoolItem.cc
ResPool.cc
NameKindProxy.cc
ResPoolProxy.cc
ResPoolManager.cc
Resolver.cc
ResolverProblem.cc
ProblemSolution.cc
Package.cc
Pathname.cc
Source.cc
SourceFactory.cc
SourceManager.cc
SourceCache.cc
SourceFeed.cc
SrcPackage.cc
Target.cc
Selection.cc
Pattern.cc
Message.cc
Script.cc
Patch.cc
Product.cc
Changelog.cc
ExternalProgram.cc
PathInfo.cc
Digest.cc
Url.cc
TmpPath.cc
DiskUsage.cc
DiskUsageCounter.cc
VendorAttr.cc
UpgradeStatistics.cc
ZYpp.cc
ZYppCommitPolicy.cc
ZYppCommitResult.cc
TranslatedText.cc
ZYppFactory.cc
PublicKey.cc
Signature.cc
KeyRing.cc
MediaSetAccess.cc
)

SET_SOURCE_FILES_PROPERTIES(${zypp_SRCS} PROPERTIES COMPILE_FLAGS -DZYPP_BASE_LOGGER_LOGGROUP=\\\"zypp\\\")

####################################################################
# SUBDIRECTORIES                                                   #
####################################################################

#add_subdir_sources(base zypp_SRCS)
#add_subdir_sources(source zypp_SRCS)
#add_subdir_sources(source/yum zypp_SRCS)

ADD_SUBDIRECTORY( base ) 
ADD_SUBDIRECTORY( thread ) 
ADD_SUBDIRECTORY( url ) 
ADD_SUBDIRECTORY( media ) 
ADD_SUBDIRECTORY( capability ) 
ADD_SUBDIRECTORY( detail ) 
ADD_SUBDIRECTORY( pool ) 
ADD_SUBDIRECTORY( parser ) 
ADD_SUBDIRECTORY( data ) 
ADD_SUBDIRECTORY( source ) 
ADD_SUBDIRECTORY( target ) 
ADD_SUBDIRECTORY( solver ) 
ADD_SUBDIRECTORY( zypp_detail ) 
ADD_SUBDIRECTORY( ui ) 


#MESSAGE( STATUS ${zypp_SRCS} )

ADD_LIBRARY(zypp SHARED ${zypp_SRCS})
TARGET_LINK_LIBRARIES(zypp boost_filesystem util )
TARGET_LINK_LIBRARIES(zypp zypp_base)
TARGET_LINK_LIBRARIES(zypp zypp_thread)
TARGET_LINK_LIBRARIES(zypp zypp_url)
TARGET_LINK_LIBRARIES(zypp zypp_media)
TARGET_LINK_LIBRARIES(zypp zypp_capability)
TARGET_LINK_LIBRARIES(zypp zypp_detail)
TARGET_LINK_LIBRARIES(zypp zypp_pool)
TARGET_LINK_LIBRARIES(zypp zypp_parser)
TARGET_LINK_LIBRARIES(zypp zypp_data)
TARGET_LINK_LIBRARIES(zypp zypp_source)
TARGET_LINK_LIBRARIES(zypp zypp_target)
TARGET_LINK_LIBRARIES(zypp zypp_solver)
TARGET_LINK_LIBRARIES(zypp zypp_zypp_detail)
TARGET_LINK_LIBRARIES(zypp zypp_ui)


#SET_TARGET_PROPERTIES(zypp PROPERTIES VERSION "${LIBZYPP_CURRENT}.${LIBZYPP_PATCH}.${LIBZYPP_AGE}" SOVERSION ${LIBZYPP_CURRENT} )

####################################################################
# INSTALL                                                          #
####################################################################

#INSTALL(TARGETS zypp DESTINATION ${CMAKE_LIBRARY_PATH} )

INSTALL_FILES(  FILES  
Arch.h
Atom.h
AutoDispose.h
Bit.h
ByteCount.h
Callback.h
Capability.h
CapAndItem.h
CapFactory.h
CapFilters.h
CapMatch.h
CapMatchHelper.h
CapSetFwd.h
CapSet.h
Changelog.h
CheckSum.h
CountryCode.h
Date.h
Dependencies.h
Dep.h
Digest.h
DiskUsageCounter.h
DiskUsage.h
Edition.h
ExternalProgram.h
KeyRing.h
KVMap.h
LanguageCode.h
Language.h
Locale.h
MediaSetAccess.h
Message.h
NameKindProxy.h
NeedAType.h
NVRAD.h
NVRA.h
NVR.h
Package.h
Patch.h
PathInfo.h
Pathname.h
Pattern.h
PoolItem.h
ProblemSolution.h
ProblemTypes.h
Product.h
PublicKey.h
Range.h
RelCompare.h
Rel.h
ResFilters.h
ResObject.h
Resolvable.h
Resolver.h
ResolverProblem.h
ResPool.h
ResPoolManager.h
ResPoolProxy.h
ResStatus.h
ResStore.h
ResTraits.h
Script.h
Selection.h
Signature.h
SilentCallbacks.h
SourceCache.h
SourceFactory.h
SourceFeed.h
Source.h
SourceManager.h
SrcPackage.h
SystemResObject.h
Target.h
TmpPath.h
TranslatedText.h
TriBool.h
UpgradeStatistics.h
Url.h
VendorAttr.h
ZYppCallbacks.h
ZYppCommit.h
ZYppCommitPolicy.h
ZYppCommitResult.h
ZYppFactory.h
ZYpp.h 
DESTINATON ${CMAKE_INSTALL_PREFIX}/include/zypp
)



#original Makefile.am contents follow:

### Process this file with automake to produce Makefile.in
### ##################################################
#
#SUBDIRS = base thread url media capability detail pool parser \
#	data source target solver zypp_detail ui
#
#AM_CXXFLAGS = -DZYPP_BASE_LOGGER_LOGGROUP=\"zypp\"
#
### ##################################################
#
#pkginclude_HEADERS = NeedAType.h \
#	Arch.h		\
#	Atom.h		\
#	AutoDispose.h	\
#	Bit.h		\
#	ByteCount.h	\
#	CapAndItem.h	\
#	Capability.h	\
#	CapFactory.h	\
#	CapMatch.h	\
#	CapSet.h	\
#	CapSetFwd.h	\
#	CapFilters.h	\
#	CountryCode.h	\
#	Date.h		\
#	Dep.h		\
#	Dependencies.h	\
#	Edition.h	\
#	Language.h	\
#	LanguageCode.h	\
#	Locale.h	\
#	NVR.h		\
#	NVRA.h		\
#	NVRAD.h		\
#	Range.h		\
#	Rel.h		\
#	RelCompare.h	\
#	ResObject.h	\
#	SystemResObject.h	\
#	Resolvable.h	\
#	ResTraits.h	\
#	ResStatus.h	\
#	ResStore.h	\
#	ResFilters.h	\
#	PoolItem.h	\
#	ResPool.h	\
#	CapMatchHelper.h\
#	NameKindProxy.h	\
#	ResPoolProxy.h	\
#	ResPoolManager.h\
#	Resolver.h	\
#	ResolverProblem.h	\
#	ProblemTypes.h	\
#	ProblemSolution.h	\
#	Package.h	\
#	Pathname.h	\
#	Source.h	\
#	SourceFactory.h	\
#	SourceManager.h	\
#	SourceCache.h	\
#	SourceFeed.h	\
#	SrcPackage.h	\
#	Target.h	\
#	TriBool.h	\
#	Selection.h	\
#	Pattern.h	\
#	Message.h	\
#	Script.h	\
#	Patch.h		\
#	Product.h	\
#	Changelog.h	\
#	ExternalProgram.h\
#	Pathname.h	\
#	PathInfo.h	\
#	Digest.h	\
#	Url.h		\
#	TmpPath.h	\
#	KVMap.h		\
#	Callback.h	\
#	DiskUsage.h	\
#	DiskUsageCounter.h	\
#	CheckSum.h	\
#	VendorAttr.h	\
#	UpgradeStatistics.h	\
#	TranslatedText.h \
#	ZYpp.h		\
#	ZYppCommit.h		\
#	ZYppCommitPolicy.h	\
#	ZYppCommitResult.h	\
#	ZYppFactory.h	\
#	ZYppCallbacks.h	\
#	SilentCallbacks.h \
#	PublicKey.h \
#	Signature.h \
#	KeyRing.h \
#	MediaSetAccess.h
#
### ##################################################
#
#lib_LTLIBRARIES =	libZYPP.la
#
### ##################################################
#
#libZYPP_la_SOURCES = \
#	LanguageCode.cc	\
#	Arch.cc		\
#	Atom.cc		\
#	ByteCount.cc	\
#	Capability.cc	\
#	CapFactory.cc	\
#	CapMatch.cc	\
#	CapSet.cc	\
#	CheckSum.cc \
#	CountryCode.cc	\
#	Date.cc		\
#	Dep.cc		\
#	Dependencies.cc	\
#	Edition.cc	\
#	Language.cc	\
#	Locale.cc	\
#	NVR.cc		\
#	NVRA.cc		\
#	NVRAD.cc	\
#	Range.cc	\
#	Rel.cc		\
#	ResObject.cc	\
#	SystemResObject.cc	\
#	Resolvable.cc	\
#	ResTraits.cc	\
#	ResStatus.cc	\
#	ResStore.cc	\
#	PoolItem.cc	\
#	ResPool.cc	\
#	NameKindProxy.cc	\
#	ResPoolProxy.cc	\
#	ResPoolManager.cc\
#	Resolver.cc	\
#	ResolverProblem.cc	\
#	ProblemSolution.cc	\
#	Package.cc	\
#	Pathname.cc	\
#	Source.cc	\
#	SourceFactory.cc\
#	SourceManager.cc\
#	SourceCache.cc	\
#	SourceFeed.cc	\
#	SrcPackage.cc	\
#	Target.cc	\
#	Selection.cc	\
#	Pattern.cc	\
#	Message.cc	\
#	Script.cc	\
#	Patch.cc	\
#	Product.cc	\
#	Changelog.cc	\
#	ExternalProgram.cc\
#	PathInfo.cc	\
#	Digest.cc	\
#	Url.cc		\
#	TmpPath.cc	\
#	DiskUsage.cc	\
#	DiskUsageCounter.cc	\
#	VendorAttr.cc	\
#	UpgradeStatistics.cc	\
#	ZYpp.cc		\
#	ZYppCommitPolicy.cc	\
#	ZYppCommitResult.cc	\
#	TranslatedText.cc \
#	ZYppFactory.cc \
#	PublicKey.cc \
#	Signature.cc \
#	KeyRing.cc \
#	MediaSetAccess.cc
#
#
#libZYPP_la_LDFLAGS =	@LIBZYPP_VERSION_INFO@
#
#libZYPP_la_LIBADD =        thread/libZYPP_thread.la		\
#				detail/libZYPP_detail.la		\
#				capability/libZYPP_capability.la	\
#				pool/libZYPP_pool.la		\
#				parser/libZYPP_parser.la		\
#				source/libZYPP_source.la		\
#				data/libZYPP_data.la		\
#				media/libZYPP_media.la		\
#				url/libZYPP_url.la			\
#				target/libZYPP_target.la		\
#				solver/libZYPP_solver.la		\
#				zypp_detail/libZYPP_zypp_detail.la	\
#				ui/libZYPP_ui.la			\
#				\
#				base/libZYPP_base.la		\
#				-lboost_filesystem			\
#				-lutil
## !!! base/libZYPP_base.la MUST BE THE LAST zypp lib linked
#
### ##################################################
#
#EXTRA_DIST = @DOXYGEN/*
#
##<INSTALL-HEADER-TARGET>
#
## Automatically inserted by insert_install_target
## DO NOT REMOVE THE TAGS!
#
## Install header files only if different to version already installed -
## prevent unnecessary installations to keep make dependency changes
## and thus rebuilds for dependent packages as low as possible.
#
#install-pkgincludeHEADERS: $(pkginclude_HEADERS)
#	@$(NORMAL_INSTALL)
#	$(mkinstalldirs) $(DESTDIR)$(pkgincludedir)
#	@list='$(pkginclude_HEADERS)';							\
#	for p in $$list; do								\
#		if test -f "$$p"; then d= ; else d="$(srcdir)/"; fi; 			\
#		if ! cmp -s $$d$$p $(DESTDIR)$(pkgincludedir)/$$p; then			\
#			echo " $(INSTALL_DATA) $$d$$p $(DESTDIR)$(pkgincludedir)/$$p";	\
#			$(INSTALL_DATA) $$d$$p $(DESTDIR)$(pkgincludedir)/$$p;		\
#	  	fi									\
#	done
#
##</INSTALL-HEADER-TARGET>
#
