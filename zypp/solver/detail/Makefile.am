#
# Makefile.am for solver/detail
#

SUBDIRS =

INCLUDES = 						\
	-I$(top_srcdir) -Wall -fno-strict-aliasing	\
	-I/usr/include/YaST2				\
	-I/usr/include/libxml2 -I/usr/include/rpm	\
	-DG_LOG_DOMAIN=\"solver_detail\"

include_HEADERS = \
	debug.h				\
	utils.h				\
	extract.h			\
	Arch.h				\
	Edition.h			\
	EditionPtr.h			\
	Hash.h				\
	Spec.h				\
	SpecPtr.h			\
	Section.h			\
	Channel.h			\
	ChannelPtr.h			\
	Subscription.h			\
	ResItem.h			\
	ResItemPtr.h			\
	Dependency.h			\
	DependencyPtr.h			\
	ResItemAndDependency.h	\
	ResItemAndDependencyPtr.h	\
	OrDependency.h			\
	OrDependencyPtr.h		\
	Package.h			\
	PackagePtr.h			\
	PackageUpdate.h			\
	PackageUpdatePtr.h		\
	XmlNode.h			\
	XmlNodePtr.h			\
	XmlParser.h			\
	Importance.h			\
	Version.h			\
	Match.h				\
	MatchPtr.h			\
	Pending.h			\
	PendingPtr.h			\
	Packman.h			\
	PackmanPtr.h			\
	World.h				\
	WorldPtr.h			\
	UndumpWorld.h			\
	UndumpWorldPtr.h		\
	StoreWorld.h			\
	StoreWorldPtr.h			\
	MultiWorld.h			\
	MultiWorldPtr.h			\
	ServiceWorld.h			\
	ServiceWorldPtr.h		\
	ResolverContext.h		\
	ResolverContextPtr.h		\
	ResolverInfo.h			\
	ResolverInfoPtr.h		\
	ResolverInfoContainer.h		\
	ResolverInfoContainerPtr.h	\
	ResolverInfoChildOf.h		\
	ResolverInfoChildOfPtr.h	\
	ResolverInfoConflictsWith.h	\
	ResolverInfoConflictsWithPtr.h	\
	ResolverInfoDependsOn.h		\
	ResolverInfoDependsOnPtr.h	\
	ResolverInfoMisc.h		\
	ResolverInfoMiscPtr.h		\
	ResolverInfoMissingReq.h	\
	ResolverInfoMissingReqPtr.h	\
	ResolverInfoNeededBy.h		\
	ResolverInfoNeededByPtr.h	\
	ResolverInfoObsoletes.h		\
	ResolverInfoObsoletesPtr.h	\
	QueueItem.h			\
	QueueItemPtr.h			\
	QueueItemBranch.h		\
	QueueItemBranchPtr.h		\
	QueueItemConflict.h		\
	QueueItemConflictPtr.h		\
	QueueItemGroup.h		\
	QueueItemGroupPtr.h		\
	QueueItemInstall.h		\
	QueueItemInstallPtr.h		\
	QueueItemRequire.h		\
	QueueItemRequirePtr.h		\
	QueueItemUninstall.h		\
	QueueItemUninstallPtr.h		\
	ResolverQueue.h			\
	ResolverQueuePtr.h		\
	Resolver.h			\
	ResolverPtr.h

noinst_LTLIBRARIES =	lib@PACKAGE@_solver_detail.la

lib@PACKAGE@_solver_detail_la_SOURCES =			\
	utils.cc					\
	debug.cc					\
	Arch.cc						\
	Edition.cc					\
	Spec.cc						\
	Channel.cc					\
	Subscription.cc					\
	ResItem.cc					\
	Dependency.cc					\
	ResItemAndDependency.cc			\
	OrDependency.cc					\
	XmlNode.cc					\
	Importance.cc					\
	Section.cc					\
	Package.cc					\
	PackageUpdate.cc				\
	XmlParser.cc					\
	Version.cc					\
	Match.cc					\
	Pending.cc					\
	Packman.cc					\
	World.cc					\
	UndumpWorld.cc					\
	StoreWorld.cc					\
	MultiWorld.cc					\
	ServiceWorld.cc					\
	QueueItem.cc					\
	QueueItemBranch.cc				\
	QueueItemConflict.cc				\
	QueueItemGroup.cc				\
	QueueItemInstall.cc				\
	QueueItemRequire.cc				\
	QueueItemUninstall.cc				\
	Resolver.cc					\
	ResolverInfo.cc					\
	ResolverInfoChildOf.cc				\
	ResolverInfoConflictsWith.cc			\
	ResolverInfoContainer.cc			\
	ResolverInfoDependsOn.cc			\
	ResolverInfoMisc.cc				\
	ResolverInfoMissingReq.cc			\
	ResolverInfoNeededBy.cc				\
	ResolverInfoObsoletes.cc			\
	ResolverContext.cc				\
	ResolverQueue.cc				\
	extract.cc

